/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "filter_encode_dds.hpp"

#include "todds/dds.hpp"

#include <cassert>
#include <limits>

#if defined(TODDS_PIPELINE_DUMP)
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/nowide/fstream.hpp>
#endif // defined(TODDS_PIPELINE_DUMP)

namespace todds::pipeline::impl {

static dds_data create_dds_data(dds_image image, std::size_t file_index) {
	dds_data data{std::move(image), file_index};
#if defined(TODDS_PIPELINE_DUMP)
	const auto dmp_path = boost::dll::program_location().parent_path() / "encode_dds.dmp";
	boost::nowide::ofstream dmp{dmp_path, std::ios::out | std::ios::binary};
	const std::uint64_t* image_start = data.image.data();
	dmp.write(
		reinterpret_cast<const char*>(image_start), static_cast<std::ptrdiff_t>(data.image.size() * sizeof(std::uint64_t)));
#endif // defined(TODDS_PIPELINE_DUMP)
	return data;
}

class encode_bc1_image final {
public:
	encode_bc1_image(vector<file_data>& files_data, const todds::format::quality quality, const bool alpha_black) noexcept
		: _files_data{files_data}
		, _quality{quality}
		, _alpha_black{alpha_black} {}

	dds_data operator()(const pixel_block_data& pixel_data) const {
		if (pixel_data.file_index == error_file_index) [[unlikely]] { return {{}, error_file_index}; }
		_files_data[pixel_data.file_index].format = todds::format::type::bc1;
		return create_dds_data(todds::dds::bc1_encode(_quality, _alpha_black, pixel_data.image), pixel_data.file_index);
	}

private:
	vector<file_data>& _files_data;
	todds::format::quality _quality;
	bool _alpha_black;
};

class encode_bc7_image final {
public:
	encode_bc7_image(vector<file_data>& files_data, todds::format::quality quality) noexcept
		: _files_data{files_data}
		, _params{todds::dds::bc7_encode_params(quality)} {}

	dds_data operator()(const pixel_block_data& pixel_data) const {
		if (pixel_data.file_index == error_file_index) [[unlikely]] { return {{}, error_file_index}; }
		_files_data[pixel_data.file_index].format = todds::format::type::bc7;
		return create_dds_data(todds::dds::bc7_encode(_params, pixel_data.image), pixel_data.file_index);
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
	explicit encode_bc1_alpha_bc7_image(
		vector<file_data>& files_data, const todds::format::quality quality, const bool alpha_black) noexcept
		: _files_data{files_data}
		, _params{todds::dds::bc7_encode_params(quality)}
		, _quality{quality}
		, _alpha_black{alpha_black} {}

	dds_data operator()(const pixel_block_data& pixel_data) const {
		if (pixel_data.file_index == error_file_index) [[unlikely]] { return {{}, error_file_index}; }
		const auto format = has_alpha(pixel_data.image) ? todds::format::type::bc7 : todds::format::type::bc1;
		_files_data[pixel_data.file_index].format = format;

		return create_dds_data(format == todds::format::type::bc7 ?
														 todds::dds::bc7_encode(_params, pixel_data.image) :
														 todds::dds::bc1_encode(_quality, _alpha_black, pixel_data.image),
			pixel_data.file_index);
	}

private:
	vector<file_data>& _files_data;
	dds::bc7_params _params;
	format::quality _quality;
	bool _alpha_black;
};

oneapi::tbb::filter<pixel_block_data, dds_data> encode_dds_filter(vector<file_data>& files_data,
	todds::format::type format_type, const todds::format::quality quality, const bool alpha_black) {
	using oneapi::tbb::filter_mode;
	using oneapi::tbb::make_filter;
	switch (format_type) {
	case todds::format::type::bc1:
		return make_filter<pixel_block_data, dds_data>(
			filter_mode::parallel, encode_bc1_image{files_data, quality, alpha_black});
	case todds::format::type::bc7:
		return make_filter<pixel_block_data, dds_data>(filter_mode::parallel, encode_bc7_image{files_data, quality});
	case todds::format::type::bc1_alpha_bc7:
		return make_filter<pixel_block_data, dds_data>(
			filter_mode::parallel, encode_bc1_alpha_bc7_image{files_data, quality, alpha_black});
	}
	assert(false);
	return {};
}

} // namespace todds::pipeline::impl
