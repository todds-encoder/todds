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

namespace {

todds::pipeline::impl::dds_data create_dds_data(todds::dds_image image, std::size_t file_index) {
	todds::pipeline::impl::dds_data data{std::move(image), file_index};
#if defined(TODDS_PIPELINE_DUMP)
	const auto dmp_path = boost::dll::program_location().parent_path() / "encode_dds.dmp";
	boost::nowide::ofstream dmp{dmp_path, std::ios::out | std::ios::binary};
	const std::uint64_t* image_start = data.image.data();
	dmp.write(
		reinterpret_cast<const char*>(image_start), static_cast<std::ptrdiff_t>(data.image.size() * sizeof(std::uint64_t)));
#endif // defined(TODDS_PIPELINE_DUMP)
	return data;
}

// When using alpha_format, this function determines if a file should be encoded as alpha.
bool has_alpha(const todds::pixel_block_image& img) {
	const auto* current_alpha = reinterpret_cast<const std::uint8_t*>(img.data()) + 3U;
	const auto* end = reinterpret_cast<const std::uint8_t*>(&img.back());

	while (current_alpha <= end) {
		if (*current_alpha != std::numeric_limits<std::uint8_t>::max()) { return true; }
		current_alpha += todds::image::bytes_per_pixel;
	}

	return false;
}

} // Anonymous namespace

namespace todds::pipeline::impl {

class encode_bc1_image final {
public:
	encode_bc1_image(vector<file_data>& files_data, const format::quality quality, const bool alpha_black) noexcept
		: _files_data{files_data}
		, _quality{quality}
		, _alpha_black{alpha_black} {}

	dds_data operator()(const pixel_block_data& pixel_data) const {
		if (pixel_data.file_index == error_file_index) [[unlikely]] { return {{}, error_file_index}; }
		_files_data[pixel_data.file_index].format = format::type::bc1;
		return create_dds_data(dds::bc1_encode(_quality, _alpha_black, pixel_data.image), pixel_data.file_index);
	}

private:
	vector<file_data>& _files_data;
	format::quality _quality;
	bool _alpha_black;
};

class encode_bc7_image final {
public:
	encode_bc7_image(vector<file_data>& files_data, format::quality quality) noexcept
		: _files_data{files_data}
		, _params{dds::bc7_encode_params(quality)} {}

	dds_data operator()(const pixel_block_data& pixel_data) const {
		if (pixel_data.file_index == error_file_index) [[unlikely]] { return {{}, error_file_index}; }
		_files_data[pixel_data.file_index].format = format::type::bc7;
		return create_dds_data(dds::bc7_encode(_params, pixel_data.image), pixel_data.file_index);
	}

private:
	vector<file_data>& _files_data;
	dds::bc7_params _params;
};

class encode_alpha_format_image final {
public:
	explicit encode_alpha_format_image(vector<file_data>& files_data, const format::type format,
		const format::type alpha_format, const format::quality quality, const bool alpha_black) noexcept
		: _files_data{files_data}
		, _format{format}
		, _alpha_format{alpha_format}
		, _params{dds::bc7_encode_params(quality)}
		, _quality{quality}
		, _alpha_black{alpha_black} {
		assert(_alpha_format != format::type::invalid);
	}

	dds_data operator()(const pixel_block_data& pixel_data) const {
		if (pixel_data.file_index == error_file_index) [[unlikely]] { return {{}, error_file_index}; }
		const auto format = has_alpha(pixel_data.image) ? _alpha_format : _format;
		_files_data[pixel_data.file_index].format = format;

		vector<std::uint64_t> image_data;
		switch (format) {
		case format::type::bc1: image_data = dds::bc1_encode(_quality, _alpha_black, pixel_data.image); break;
		case format::type::bc7: image_data = dds::bc7_encode(_params, pixel_data.image); break;
		case format::type::invalid: assert(false); break;
		}

		return create_dds_data(image_data, pixel_data.file_index);
	}

private:
	vector<file_data>& _files_data;
	format::type _format;
	format::type _alpha_format;
	dds::bc7_params _params;
	format::quality _quality;
	bool _alpha_black;
};

oneapi::tbb::filter<pixel_block_data, dds_data> encode_dds_filter(vector<file_data>& files_data, format::type format,
	format::type alpha_format, const format::quality quality, const bool alpha_black) {
	using oneapi::tbb::filter_mode;
	using oneapi::tbb::make_filter;

	if (alpha_format != format::type::invalid) {
		return make_filter<pixel_block_data, dds_data>(
			filter_mode::parallel, encode_alpha_format_image{files_data, format, alpha_format, quality, alpha_black});
	}

	switch (format) {
	case format::type::bc1:
		return make_filter<pixel_block_data, dds_data>(
			filter_mode::parallel, encode_bc1_image{files_data, quality, alpha_black});
	case format::type::bc7:
		return make_filter<pixel_block_data, dds_data>(filter_mode::parallel, encode_bc7_image{files_data, quality});
	case format::type::invalid: break;
	}

	assert(false);
	return {};
}

} // namespace todds::pipeline::impl
