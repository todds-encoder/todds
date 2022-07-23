/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/pipeline.hpp"

#include "png2dds/dds.hpp"
#include "png2dds/dds_image.hpp"
#include "png2dds/image.hpp"
#include "png2dds/pixel_block_image.hpp"
#include "png2dds/png.hpp"

#include <boost/nowide/fstream.hpp>
#include <oneapi/tbb/parallel_pipeline.h>

#include <atomic>

namespace otbb = oneapi::tbb;

using png2dds::dds_image;
using png2dds::image;
using png2dds::pixel_block_image;
using png2dds::pipeline::paths_vector;

namespace {

constexpr std::size_t error_file_index = std::numeric_limits<std::size_t>::max();

struct png_file {
	std::vector<std::uint8_t> buffer;
	std::size_t file_index;
};

class load_png_file final {
public:
	explicit load_png_file(const paths_vector& paths, std::atomic<std::size_t>& counter) noexcept
		: _paths{paths}
		, _counter{counter} {}

	png_file operator()(otbb::flow_control& flow) const {
		std::size_t index = _counter++;
		if (index >= _paths.size()) {
			flow.stop();
			return {};
		}
		boost::nowide::ifstream ifs{_paths[index].first, std::ios::in | std::ios::binary};
		return {{std::istreambuf_iterator<char>{ifs}, {}}, index};
	}

private:
	const paths_vector& _paths;
	std::atomic<std::size_t>& _counter;
};

class decode_png_image final {
public:
	explicit decode_png_image(const paths_vector& paths, bool vflip) noexcept
		: _paths{paths}
		, _vflip{vflip} {}

	image operator()(const png_file& file) const {
		try {
			return png2dds::png::decode(file.file_index, _paths[file.file_index].first.string(), file.buffer, _vflip);
		} catch (const std::runtime_error& /*ex*/) {
			// ToDo error reporting when the verbose option is activated.
		}
		return {error_file_index, 0U, 0U};
	}

private:
	const paths_vector& _paths;
	bool _vflip;
};

class get_pixel_blocks final {
public:
	pixel_block_image operator()(const image& image) const { return pixel_block_image{image}; }
};

class encode_dds_image final {
public:
	explicit encode_dds_image(unsigned int level) noexcept
		: _params{png2dds::dds::bc7_encode_params(level)} {}

	dds_image operator()(const pixel_block_image& pixel_image) const {
		return pixel_image.file_index() != error_file_index ? png2dds::dds::bc7_encode(_params, pixel_image) :
																													dds_image{pixel_image};
	}

private:
	ispc::bc7e_compress_block_params _params;
};

class save_dds_file final {
public:
	explicit save_dds_file(const paths_vector& paths) noexcept
		: _paths{paths} {}

	void operator()(const dds_image& dds_img) const {
		if (dds_img.file_index() == error_file_index) { return; }
		boost::nowide::ofstream ofs{_paths[dds_img.file_index()].second, std::ios::out | std::ios::binary};
		ofs << "DDS ";
		const std::size_t block_size_bytes = dds_img.blocks().size() * sizeof(dds_image::block_type);
		const auto header = dds_img.header();
		ofs.write(header.data(), header.size());
		ofs.write(reinterpret_cast<const char*>(dds_img.blocks().data()), static_cast<std::ptrdiff_t>(block_size_bytes));
		ofs.close();
	}

private:
	const paths_vector& _paths;
};

} // Anonymous namespace

namespace png2dds::pipeline {

void encode_as_dds(std::size_t tokens, unsigned int level, bool vflip, const paths_vector& paths) {
	// Variables referenced by the filters.
	std::atomic<std::size_t> counter;

	const otbb::filter<void, void> filters =
		otbb::make_filter<void, png_file>(otbb::filter_mode::serial_in_order, load_png_file(paths, counter)) &
		otbb::make_filter<png_file, image>(otbb::filter_mode::parallel, decode_png_image(paths, vflip)) &
		otbb::make_filter<image, pixel_block_image>(otbb::filter_mode::parallel, get_pixel_blocks{}) &
		otbb::make_filter<pixel_block_image, dds_image>(otbb::filter_mode::parallel, encode_dds_image(level)) &
		otbb::make_filter<dds_image, void>(otbb::filter_mode::parallel, save_dds_file(paths));

	otbb::parallel_pipeline(tokens, filters);
}
} // namespace png2dds::pipeline