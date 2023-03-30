/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "filter_encode_dds.hpp"

#include "todds/dds.hpp"

#include <cassert>
#include <limits>

namespace todds::pipeline::impl {

class encode_bc1_image final {
public:
	encode_bc1_image(vector<file_data>& files_data, todds::format::quality quality) noexcept
		: _files_data{files_data}
		, _quality{quality} {}

	dds_data operator()(const pixel_block_data& pixel_data) const {
		TracyZoneScopedN("encode_bc1_image");
		TracyZoneFileIndex(pixel_data.file_index);
		if (pixel_data.file_index == error_file_index) [[unlikely]] { return {{}, error_file_index}; }
		_files_data[pixel_data.file_index].format = todds::format::type::bc1;
		return {todds::dds::bc1_encode(_quality, pixel_data.image), pixel_data.file_index};
	}

private:
	vector<file_data>& _files_data;
	todds::format::quality _quality;
};

class encode_bc7_image final {
public:
	encode_bc7_image(vector<file_data>& files_data, todds::format::quality quality) noexcept
		: _files_data{files_data}
		, _params{todds::dds::bc7_encode_params(quality)} {}

	dds_data operator()(const pixel_block_data& pixel_data) const {
		TracyZoneScopedN("encode_bc7_image");
		TracyZoneFileIndex(pixel_data.file_index);
		if (pixel_data.file_index == error_file_index) [[unlikely]] { return {{}, error_file_index}; }
		_files_data[pixel_data.file_index].format = todds::format::type::bc7;
		return {todds::dds::bc7_encode(_params, pixel_data.image), pixel_data.file_index};
	}

private:
	vector<file_data>& _files_data;
	dds::bc7_params _params;
};

// When using BC1_ALPHA_BC7, this function determines if a file should be encoded as BC7.
static bool has_alpha(const todds::pixel_block_image& img) {
	const auto* current_alpha = reinterpret_cast<const std::uint8_t*>(img.data()) + 3U;
	const auto* end = reinterpret_cast<const std::uint8_t*>(&img.back());

	while (current_alpha <= end) {
		if (*current_alpha != std::numeric_limits<std::uint8_t>::max()) { return true; }
		current_alpha += image::bytes_per_pixel;
	}

	return false;
}

class encode_bc1_alpha_bc7_image final {
public:
	explicit encode_bc1_alpha_bc7_image(vector<file_data>& files_data, todds::format::quality quality) noexcept
		: _files_data{files_data}
		, _params{todds::dds::bc7_encode_params(quality)}
		, _quality{quality} {}

	dds_data operator()(const pixel_block_data& pixel_data) const {
		TracyZoneScopedN("encode_bc1_alpha_bc7");
		TracyZoneFileIndex(pixel_data.file_index);
		if (pixel_data.file_index == error_file_index) [[unlikely]] { return {{}, error_file_index}; }
		const auto format = has_alpha(pixel_data.image) ? todds::format::type::bc7 : todds::format::type::bc1;
		_files_data[pixel_data.file_index].format = format;

		return {format == todds::format::type::bc7 ? todds::dds::bc7_encode(_params, pixel_data.image) :
																								 todds::dds::bc1_encode(_quality, pixel_data.image),
			pixel_data.file_index};
	}

private:
	vector<file_data>& _files_data;
	dds::bc7_params _params;
	format::quality _quality;
};

oneapi::tbb::filter<pixel_block_data, dds_data> encode_dds_filter(
	vector<file_data>& files_data, todds::format::type format_type, todds::format::quality quality) {
	using oneapi::tbb::filter_mode;
	using oneapi::tbb::make_filter;
	switch (format_type) {
	case todds::format::type::bc1:
		return make_filter<pixel_block_data, dds_data>(filter_mode::parallel, encode_bc1_image{files_data, quality});
	case todds::format::type::bc7:
		return make_filter<pixel_block_data, dds_data>(filter_mode::parallel, encode_bc7_image{files_data, quality});
	case todds::format::type::bc1_alpha_bc7:
		return make_filter<pixel_block_data, dds_data>(
			filter_mode::parallel, encode_bc1_alpha_bc7_image{files_data, quality});
	}
	assert(false);
	return {};
}

} // namespace todds::pipeline::impl
