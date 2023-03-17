/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "filter_encode_dds.hpp"

#include "png2dds/dds.hpp"

#include <cassert>

namespace png2dds::pipeline::impl {

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

// When using BC1_ALPHA_BC7, this function determines if a file should be encoded as BC7.
static bool has_alpha(const png2dds::pixel_block_image& img) {
	for (const std::uint32_t pixel : img) {
		// Pixels are encoded as RGBA.
		if ((pixel % 256) < 255) { return true; }
	}

	return false;
}

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

oneapi::tbb::filter<pixel_block_data, dds_data> encode_dds_filter(
	std::vector<file_data>& files_data, png2dds::format::type format_type, png2dds::format::quality quality) {
	using oneapi::tbb::filter_mode;
	using oneapi::tbb::make_filter;
	switch (format_type) {
	case png2dds::format::type::bc1:
		return make_filter<pixel_block_data, dds_data>(filter_mode::parallel, encode_bc1_image{files_data, quality});
	case png2dds::format::type::bc7:
		return make_filter<pixel_block_data, dds_data>(filter_mode::parallel, encode_bc7_image{files_data, quality});
	case png2dds::format::type::bc1_alpha_bc7:
		return make_filter<pixel_block_data, dds_data>(
			filter_mode::parallel, encode_bc1_alpha_bc7_image{files_data, quality});
	}
	assert(false);
	return {};
}

} // namespace png2dds::pipeline::impl
