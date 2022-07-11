/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "pipeline.hpp"

#include "png2dds/dds.hpp"
#include "png2dds/png.hpp"

#include <boost/nowide/fstream.hpp>
#include <oneapi/tbb/parallel_pipeline.h>

#include <atomic>

namespace otbb = oneapi::tbb;

using png2dds::pipeline::paths_vector;
using image = png2dds::image;
using png2dds::dds_image;

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
	explicit decode_png_image(const paths_vector& paths, bool flip) noexcept
		: _paths{paths}
		, _flip{flip} {}

	image operator()(const png_file& file) const {
		try {
			return png2dds::png::decode(file.file_index, _paths[file.file_index].first.string(), file.buffer, _flip);
		} catch (const std::runtime_error& /*ex*/) {
			// ToDo error reporting when the verbose option is activated.
		}
		return {error_file_index, 0U, 0U};
	}

private:
	const paths_vector& _paths;
	bool _flip;
};

class encode_dds_image final {
public:
	explicit encode_dds_image(const png2dds::dds::encoder& encoder) noexcept
		: _encoder{encoder} {}

	dds_image operator()(const image& png_image) const {
		return png_image.file_index() != error_file_index ? _encoder.encode(png_image) : dds_image{png_image};
	}

private:
	const png2dds::dds::encoder& _encoder;
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

void encode_as_dds(std::size_t tokens, unsigned int level, bool flip, const paths_vector& paths) {
	// Variables referenced by the filters.
	std::atomic<std::size_t> counter;
	dds::encoder encoder{level};

	const otbb::filter<void, void> filters =
		otbb::make_filter<void, png_file>(otbb::filter_mode::serial_in_order, load_png_file(paths, counter)) &
		otbb::make_filter<png_file, image>(otbb::filter_mode::parallel, decode_png_image(paths, flip)) &
		otbb::make_filter<image, dds_image>(otbb::filter_mode::parallel, encode_dds_image(encoder)) &
		otbb::make_filter<dds_image, void>(otbb::filter_mode::parallel, save_dds_file(paths));

	otbb::parallel_pipeline(tokens, filters);
}
} // namespace png2dds::pipeline