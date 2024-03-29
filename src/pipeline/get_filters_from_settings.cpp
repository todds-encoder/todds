/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "get_filters_from_settings.hpp"

#include "todds/report.hpp"

#include "filter_decode_png.hpp"
#include "filter_encode_dds.hpp"
#include "filter_encode_png.hpp"
#include "filter_generate_mipmaps.hpp"
#include "filter_load_png.hpp"
#include "filter_pixel_blocks.hpp"
#include "filter_save_dds.hpp"
#include "filter_save_png.hpp"
#include "filter_scale_image.hpp"

namespace todds::pipeline::impl {

inline oneapi::tbb::filter<void, std::unique_ptr<mipmap_image>> png_decoding_filters(const input& input_data,
	std::atomic<std::size_t>& counter, std::atomic<bool>& force_finish, report_queue& updates,
	vector<impl::file_data>& files_data) {
	// If scale and mipmaps are enabled, space for mipmaps will be allocated by the scale filter.
	const bool should_allocate_mipmaps = input_data.mipmaps && input_data.scale == 100U;
	return // Load PNG files from disk into memory.
		impl::load_png_filter(input_data.paths, counter, force_finish, updates) &
		// Decode a PNG file to raw pixels. Fix size and allocate for mipmaps if needed.
		impl::decode_png_filter(
			files_data, input_data.paths, input_data.vflip, should_allocate_mipmaps, input_data.fix_size, updates);
}

inline oneapi::tbb::filter<std::unique_ptr<mipmap_image>, void> dds_encoding_filters(
	const input& input_data, vector<impl::file_data>& files_data, report_queue& updates) {
	return
		// Convert images into pixel block images. The pixels of these images are rearranged into 4x4 blocks,
		// ready for the DDS encoding stage.
		impl::pixel_blocks_filter() &
		// Encode pixel block images as DDS files.
		impl::encode_dds_filter(
			files_data, input_data.format, input_data.alpha_format, input_data.quality, input_data.alpha_black) &
		// Save DDS files back into the file system, one by one.
		impl::save_dds_filter(files_data, input_data.paths, updates);
}

inline oneapi::tbb::filter<std::unique_ptr<mipmap_image>, void> png_encoding_filters(
	const input& input_data, report_queue& updates) {
	return impl::encode_png_filter(input_data.paths, updates) & impl::save_png_filter(input_data.paths);
}

oneapi::tbb::filter<void, void> get_filters_from_settings(const input& input_data, std::atomic<std::size_t>& counter,
	std::atomic<bool>& force_finish, report_queue& updates, vector<impl::file_data>& files_data) {
	auto prepare_image = png_decoding_filters(input_data, counter, force_finish, updates, files_data);
	if (input_data.scale != 100U || input_data.max_size > 0U) {
		prepare_image &= impl::scale_image_filter(files_data, input_data.mipmaps, input_data.scale, input_data.max_size,
			input_data.scale_filter, input_data.paths, updates);
	}

	if (input_data.format == format::type::png) { return prepare_image & png_encoding_filters(input_data, updates); }

	if (input_data.mipmaps) {
		prepare_image &= impl::generate_mipmaps_filter(input_data.mipmap_filter, input_data.mipmap_blur);
	}
	return prepare_image & dds_encoding_filters(input_data, files_data, updates);
}

} // namespace todds::pipeline::impl
