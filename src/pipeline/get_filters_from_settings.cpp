/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "get_filters_from_settings.hpp"

#include "filter_decode_png.hpp"
#include "filter_encode_dds.hpp"
#include "filter_generate_mipmaps.hpp"
#include "filter_load_png.hpp"
#include "filter_pixel_blocks.hpp"
#include "filter_save_dds.hpp"

namespace todds::pipeline::impl {

oneapi::tbb::filter<void, void> pipeline_filters(const input& input_data, std::atomic<std::size_t>& counter,
	std::atomic<bool>& force_finish, oneapi::tbb::concurrent_queue<std::string>& error_log,
	vector<impl::file_data>& files_data) {
	return // Load PNG files into memory, one by one.
		impl::load_png_filter(input_data.paths, counter, force_finish, error_log) &
		// Decode a PNG file into pixels.
		impl::decode_png_filter(files_data, input_data.paths, input_data.vflip, input_data.mipmaps, error_log) &
		// Convert images into pixel block images. The pixels of these images are rearranged into 4x4 blocks,
		// ready for the DDS encoding stage.
		impl::pixel_blocks_filter() &
		// Encode pixel block images as DDS files.
		impl::encode_dds_filter(files_data, input_data.format, input_data.quality) &
		// Save DDS files back into the file system, one by one.
		impl::save_dds_filter(files_data, input_data.paths);
}

oneapi::tbb::filter<void, void> pipeline_filters_mipmaps(const input& input_data, std::atomic<std::size_t>& counter,
	std::atomic<bool>& force_finish, oneapi::tbb::concurrent_queue<std::string>& error_log,
	vector<impl::file_data>& files_data) {
	return // Load PNG files into memory, one by one.
		impl::load_png_filter(input_data.paths, counter, force_finish, error_log) &
		// Decode a PNG file into pixels.
		impl::decode_png_filter(files_data, input_data.paths, input_data.vflip, input_data.mipmaps, error_log) &
		// Generate all mipmaps.
		impl::generate_mipmaps_filter(input_data.mipmap_filter) &
		// Convert images into pixel block images. The pixels of these images are rearranged into 4x4 blocks,
		// ready for the DDS encoding stage.
		impl::pixel_blocks_filter() &
		// Encode pixel block images as DDS files.
		impl::encode_dds_filter(files_data, input_data.format, input_data.quality) &
		// Save DDS files back into the file system, one by one.
		impl::save_dds_filter(files_data, input_data.paths);
}

oneapi::tbb::filter<void, void> get_filters_from_settings(const input& input_data, std::atomic<std::size_t>& counter,
	std::atomic<bool>& force_finish, oneapi::tbb::concurrent_queue<std::string>& error_log,
	vector<impl::file_data>& files_data) {
	const auto gen_func = input_data.mipmaps ? pipeline_filters_mipmaps : pipeline_filters;
	return gen_func(input_data, counter, force_finish, error_log, files_data);
}

} // namespace todds::pipeline::impl
