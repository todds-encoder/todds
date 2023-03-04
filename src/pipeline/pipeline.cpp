/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/pipeline.hpp"

#include "png2dds/dds.hpp"
#include "png2dds/image.hpp"
#include "png2dds/png.hpp"
#include "png2dds/vector.hpp"

#include <boost/nowide/fstream.hpp>
#include <boost/nowide/iostream.hpp>
#include <boost/predef.h>
#include <dds_defs.h>
#include <fmt/format.h>
#include <oneapi/tbb/concurrent_queue.h>
#include <oneapi/tbb/global_control.h>
#include <oneapi/tbb/parallel_pipeline.h>

#include <atomic>
#include <chrono>
#include <future>
#if BOOST_OS_WINDOWS
#include <windows.h>
#else
#include <csignal>
#endif

namespace otbb = oneapi::tbb;

using png2dds::dds_image;
using png2dds::image;
using png2dds::pixel_block_image;
using png2dds::pipeline::paths_vector;

namespace {

std::atomic<bool> force_finish = false;
otbb::concurrent_queue<std::string> error_log;

#if BOOST_OS_WINDOWS
BOOL WINAPI ctrl_c_signal(DWORD ctrl_type) {
	if (ctrl_type == CTRL_C_EVENT) {
		// If force_finish was already true, it means that Ctrl+C has been pressed a second time.
		error_log.push("Cancelling encoding...");
		const bool should_stop = force_finish.exchange(true);
		return should_stop ? FALSE : TRUE;
	}

	return FALSE;
}

void handle_ctrl_c_signal() { SetConsoleCtrlHandler(ctrl_c_signal, TRUE); }
#else
struct sigaction act;
struct sigaction oldact;

void ctrl_c_signal(int signum) {
	if (signum == SIGINT) {
		// If force_finish was already true, it means that Ctrl+C has been pressed a second time.
		error_log.push("Cancelling encoding...");
		force_finish = true;
	}
}

void handle_ctrl_c_signal() {
	act.sa_handler = ctrl_c_signal;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, &oldact);
}
#endif

// When using BC1_ALPHA_BC7, this function determines if a file should be encoded as BC7.
bool has_alpha(const pixel_block_image& img) {
	for (const std::uint32_t pixel : img) {
		if ((pixel % 256) < 255) { return true; }
	}

	return false;
}

// Files using this file index have triggered errors and should not be processed.
constexpr std::size_t error_file_index = std::numeric_limits<std::size_t>::max();

// Header extension for BC7 files.
constexpr DDS_HEADER_DXT10 header_extension{DXGI_FORMAT_BC7_UNORM, D3D10_RESOURCE_DIMENSION_TEXTURE2D, 0U, 1U, 0U};

struct file_data {
	// Width of the image excluding extra columns. Set during the decoding PNG stage.
	std::size_t width{};
	// Height of the image excluding extra rows. Set during the decoding PNG stage.
	std::size_t height{};
	// Number of mipmap levels in the image, including the main one. Set during the decode PNG stage.
	std::size_t mipmaps{};
	// DDS format of the image. Set during the encoding DDS stage.
	png2dds::format::type format{};
};

struct png_file {
	png2dds::vector<std::uint8_t> buffer;
	std::size_t file_index;
};

class load_png_file final {
public:
	explicit load_png_file(const paths_vector& paths, std::atomic<std::size_t>& counter) noexcept
		: _paths{paths}
		, _counter{counter} {}

	png_file operator()(otbb::flow_control& flow) const {
		const std::size_t index = _counter++;
		if (index >= _paths.size() || force_finish) [[unlikely]] {
			flow.stop();
			return {};
		}

#if BOOST_OS_WINDOWS
		const boost::filesystem::path input{R"(\\?\)" + _paths[index].first.string()};
#else
		const boost::filesystem::path& input{_paths[index].first};
#endif
		boost::nowide::ifstream ifs{input, std::ios::in | std::ios::binary};

		if (!ifs.is_open()) [[unlikely]] {
			error_log.push(fmt::format("Load PNG file error in {:s} ", _paths[index].first.string()));
		}

		return {{std::istreambuf_iterator<char>{ifs}, {}}, index};
	}

private:
	const paths_vector& _paths;
	std::atomic<std::size_t>& _counter;
};

class decode_png_image final {
public:
	explicit decode_png_image(std::vector<file_data>& files_data, const paths_vector& paths, bool vflip) noexcept
		: _files_data{files_data}
		, _paths{paths}
		, _vflip{vflip} {}

	image operator()(const png_file& file) const {
		image result{error_file_index, 0U, 0U};

		// If the buffer is empty, assume that load_png_file already reported an error.
		if (!file.buffer.empty()) {
			const std::string& path = _paths[file.file_index].first.string();
			try {
				auto& file_data = _files_data[file.file_index];
				result = png2dds::png::decode(file.file_index, path, file.buffer, _vflip, file_data.width, file_data.height);
			} catch (const std::runtime_error& exc) {
				error_log.push(fmt::format("PNG Decoding error {:s} -> {:s}", path, exc.what()));
			}
		}

		return result;
	}

private:
	std::vector<file_data>& _files_data;
	const paths_vector& _paths;
	bool _vflip;
};

struct pixel_block_data {
	pixel_block_image image;
	std::size_t file_index;
};

class get_pixel_blocks final {
public:
	explicit get_pixel_blocks(std::vector<file_data>& files_data) noexcept
		: _files_data{files_data} {}

	pixel_block_data operator()(const image& image) const {
		const auto& file_data = _files_data[image.file_index()];
		return pixel_block_data{png2dds::to_pixel_blocks(image, file_data.width, file_data.height), image.file_index()};
	}

private:
	std::vector<file_data>& _files_data;
};

struct dds_data {
	dds_image image;
	std::size_t file_index;
};

class encode_bc1_image final {
public:
	encode_bc1_image(std::vector<file_data>& files_data, png2dds::format::quality quality) noexcept
		: _files_data{files_data}
		, _quality{quality} {}

	dds_data operator()(const pixel_block_data& pixel_data) const {
		_files_data[pixel_data.file_index].format = png2dds::format::type::bc1;
		return {png2dds::dds::bc1_encode(_quality, pixel_data.image), pixel_data.file_index};
	}

private:
	std::vector<file_data>& _files_data;
	png2dds::format::quality _quality;
};

class encode_bc7_image final {
public:
	encode_bc7_image(std::vector<file_data>& files_data, png2dds::format::quality quality) noexcept
		: _files_data{files_data}
		, _params{png2dds::dds::bc7_encode_params(quality)} {}

	dds_data operator()(const pixel_block_data& pixel_data) const {
		_files_data[pixel_data.file_index].format = png2dds::format::type::bc7;
		return {png2dds::dds::bc7_encode(_params, pixel_data.image), pixel_data.file_index};
	}

private:
	std::vector<file_data>& _files_data;
	png2dds::dds::bc7_params _params;
};

class encode_bc1_alpha_bc7_image final {
public:
	explicit encode_bc1_alpha_bc7_image(std::vector<file_data>& files_data, png2dds::format::quality quality) noexcept
		: _files_data{files_data}
		, _params{png2dds::dds::bc7_encode_params(quality)}
		, _quality{quality} {}

	dds_data operator()(const pixel_block_data& pixel_data) const {
		const auto format = has_alpha(pixel_data.image) ? png2dds::format::type::bc7 : png2dds::format::type::bc1;
		_files_data[pixel_data.file_index].format = format;

		return {format == png2dds::format::type::bc7 ? png2dds::dds::bc7_encode(_params, pixel_data.image) :
																									 png2dds::dds::bc1_encode(_quality, pixel_data.image),
			pixel_data.file_index};
	}

private:
	std::vector<file_data>& _files_data;
	png2dds::dds::bc7_params _params;
	png2dds::format::quality _quality;
};

class save_dds_file final {
public:
	explicit save_dds_file(const std::vector<file_data>& files_data, const paths_vector& paths) noexcept
		: _files_data{files_data}
		, _paths{paths} {}

	void operator()(const dds_data& dds_img) const {
		const std::size_t file_index = dds_img.file_index;
		if (dds_img.file_index == error_file_index) [[unlikely]] { return; }

#if BOOST_OS_WINDOWS
		const boost::filesystem::path output{R"(\\?\)" + _paths[file_index].second.string()};
#else
		const boost::filesystem::path& output{_paths[file_index].second.string()};
#endif

		boost::nowide::ofstream ofs{output, std::ios::out | std::ios::binary};

		ofs << "DDS ";
		const std::size_t block_size_bytes = dds_img.image.size() * sizeof(std::uint64_t);
		const auto& file_data = _files_data[file_index];
		const auto header = png2dds::dds::dds_header(
			file_data.format, file_data.width, file_data.height, block_size_bytes, file_data.mipmaps);
		ofs.write(header.data(), header.size());
		if (file_data.format == png2dds::format::type::bc7) {
			ofs.write(reinterpret_cast<const char*>(&header_extension), sizeof(header_extension));
		}

		ofs.write(reinterpret_cast<const char*>(dds_img.image.data()), static_cast<std::ptrdiff_t>(block_size_bytes));
		ofs.close();
	}

private:
	const std::vector<file_data>& _files_data;
	const paths_vector& _paths;
};

void error_reporting(std::atomic<std::size_t>& progress, std::size_t total) {
	using namespace std::chrono_literals;
	std::size_t last_progress{};
	std::string error_str;
	bool requires_newline = true;

	while (progress < total && !force_finish) {
		while (error_log.try_pop(error_str)) {
			if (requires_newline) {
				boost::nowide::cerr << '\n';
				requires_newline = false;
			}
			boost::nowide::cout.flush();
			boost::nowide::cerr << error_str << '\n';
			boost::nowide::cerr.flush();
		}
		const std::size_t current_progress = progress;

		if (current_progress > last_progress && current_progress < total) {
			last_progress = current_progress;
			boost::nowide::cout << fmt::format("\rProgress: {:d}/{:d}", current_progress, total);
			boost::nowide::cout.flush();
			requires_newline = true;
			std::this_thread::sleep_for(50ms);
		}
	}
}

otbb::filter<pixel_block_data, dds_data> encoding_filter(
	std::vector<file_data>& files_data, png2dds::format::type format_type, png2dds::format::quality quality) {
	switch (format_type) {
	case png2dds::format::type::bc1:
		return otbb::make_filter<pixel_block_data, dds_data>(
			otbb::filter_mode::parallel, encode_bc1_image{files_data, quality});
	case png2dds::format::type::bc7:
		return otbb::make_filter<pixel_block_data, dds_data>(
			otbb::filter_mode::parallel, encode_bc7_image{files_data, quality});
	case png2dds::format::type::bc1_alpha_bc7:
		return otbb::make_filter<pixel_block_data, dds_data>(
			otbb::filter_mode::parallel, encode_bc1_alpha_bc7_image{files_data, quality});
	}
	assert(false);
	return {};
}

} // Anonymous namespace

namespace png2dds::pipeline {

void encode_as_dds(const input& input_data) {
	// Initialize encoders.
	switch (input_data.format) {
	case format::type::bc1: dds::initialize_bc1_encoding(); break;
	case format::type::bc7: dds::initialize_bc7_encoding(); break;
	case format::type::bc1_alpha_bc7:
		dds::initialize_bc1_encoding();
		dds::initialize_bc7_encoding();
		break;
	}

	// Setup the pipeline.
	const otbb::global_control control(otbb::global_control::max_allowed_parallelism, input_data.parallelism);
	// Maximum number of files that the pipeline can process at the same time.
	const std::size_t tokens = input_data.parallelism * 4UL;

	// Used to give each file processed in a token a unique file index to access the paths and files_data vectors.
	std::atomic<std::size_t> counter;
	// Contains extra data about each file being processed.
	// Pipeline stages may write or read from this vector at any time. Since each token has a unique index, these
	// accesses are thread-safe.
	std::vector<file_data> files_data(input_data.paths.size());

	std::future<void> error_report{};
	if (input_data.verbose) {
		error_report = std::async(std::launch::async, error_reporting, std::ref(counter), input_data.paths.size());
	}

	const otbb::filter<void, void> filters =
		// Load PNG files into memory, one by one.
		otbb::make_filter<void, png_file>(otbb::filter_mode::serial_in_order, load_png_file(input_data.paths, counter)) &
		// Decode PNG files into images loaded in memory.
		otbb::make_filter<png_file, image>(
			otbb::filter_mode::parallel, decode_png_image(files_data, input_data.paths, input_data.vflip)) &
		// Convert images into pixel block images. The pixels of these images are rearranged into 4x4 blocks,
		// ready for the DDS encoding stage.
		otbb::make_filter<image, pixel_block_data>(otbb::filter_mode::parallel, get_pixel_blocks(files_data)) &
		// Encode pixel block images as DDS files.
		encoding_filter(files_data, input_data.format, input_data.quality) &
		// Save DDS files back into the file system, one by one.
		otbb::make_filter<dds_data, void>(otbb::filter_mode::parallel, save_dds_file{files_data, input_data.paths});

	// Sending the Ctrl+C signal will stop loading new files, but files being currently processed will carry on.
	handle_ctrl_c_signal();
	otbb::parallel_pipeline(tokens, filters);

	if (error_report.valid()) {
		// Wait until the error report task is done before finishing the error log report.
		error_report.get();
		std::string error_str;
		while (error_log.try_pop(error_str)) { boost::nowide::cerr << error_str << '\n'; }
		boost::nowide::cerr.flush();
		const auto paths_size = input_data.paths.size();
		boost::nowide::cout << fmt::format("\rProgress: {:d}/{:d}\n", std::min(counter.load(), paths_size), paths_size);
		boost::nowide::cout.flush();
	}
}

} // namespace png2dds::pipeline
