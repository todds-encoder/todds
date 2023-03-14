/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "filter_decode_png.hpp"

#include "png2dds/mipmap_image.hpp"
#include "png2dds/png.hpp"
#include "png2dds/resize.hpp"

#include <fmt/format.h>

namespace {

void add_padding(png2dds::image& img) {
	// When padding has been added to the image, copy the border pixel into the padding.
	const auto width = img.width();
	const auto height = img.height();
	const auto padded_width = img.padded_width();
	const auto padded_height = img.padded_height();
	if (width < padded_width) {
		const auto border_pixel_x = width - 1UL;
		for (std::size_t pixel_y = 0UL; pixel_y < padded_height; ++pixel_y) {
			const auto border_pixel = img.get_pixel(border_pixel_x, std::min(pixel_y, height - 1U));
			for (std::size_t pixel_x = width; pixel_x < padded_width; ++pixel_x) {
				const auto current_pixel = img.get_pixel(pixel_x, pixel_y);
				std::copy(border_pixel.begin(), border_pixel.end(), current_pixel.begin());
			}
		}
	}

	if (height < padded_height) {
		const auto border_pixel_y = height - 1UL;
		for (std::size_t pixel_x = 0UL; pixel_x < padded_width; ++pixel_x) {
			const auto border_pixel = img.get_pixel(std::min(pixel_x, width - 1U), border_pixel_y);
			for (std::size_t pixel_y = height; pixel_y < padded_height; ++pixel_y) {
				const auto current_pixel = img.get_pixel(pixel_x, pixel_y);
				std::copy(border_pixel.begin(), border_pixel.end(), current_pixel.begin());
			}
		}
	}
}

void process_image(png2dds::mipmap_image& mipmap_img) {
	add_padding(mipmap_img.get_image(0UL));

	for (std::size_t mipmap_index = 1ULL; mipmap_index < mipmap_img.mipmap_count(); ++mipmap_index) {
		png2dds::image& current_image = mipmap_img.get_image(mipmap_index);
		// ToDo Support for additional downscaling algorithms.
		png2dds::box_downscale(mipmap_img.get_image(mipmap_index - 1UL), current_image);
		// ToDo preserve alpha coverage.
		add_padding(current_image);
	}
}

} // Anonymous namespace

namespace png2dds::pipeline::impl {

class decode_png final {
public:
	explicit decode_png(std::vector<file_data>& files_data, const paths_vector& paths, bool vflip, bool mipmaps,
		error_queue& errors) noexcept
		: _files_data{files_data}
		, _paths{paths}
		, _vflip{vflip}
		, _errors{errors}
		, _mipmaps{mipmaps} {}

	mipmap_image operator()(const png_file& file) const {
		mipmap_image result{error_file_index, 0U, 0U, false};

		// If the data is empty, assume that load_png_file already reported an error.
		if (!file.buffer.empty()) {
			const std::string& path = _paths[file.file_index].first.string();
			try {
				auto& file_data = _files_data[file.file_index];
				// Load the first image of the mipmap image and reserve the memory for the rest of the images.
				result = png::decode(file.file_index, path, file.buffer, _vflip, file_data.width, file_data.height, _mipmaps);
				file_data.mipmaps = result.mipmap_count();
				// Add padding, calculate mipmaps, etc.
				process_image(result);
			} catch (const std::runtime_error& exc) {
				_errors.push(fmt::format("PNG Decoding error {:s} -> {:s}", path, exc.what()));
			}
		}

		return result;
	}

private:
	std::vector<file_data>& _files_data;
	const paths_vector& _paths;
	bool _vflip;
	error_queue& _errors;
	bool _mipmaps;
};

oneapi::tbb::filter<png_file, mipmap_image> decode_png_filter(
	std::vector<file_data>& files_data, const paths_vector& paths, bool vflip, bool mipmaps, error_queue& errors) {
	return oneapi::tbb::make_filter<png_file, mipmap_image>(
		oneapi::tbb::filter_mode::serial_in_order, decode_png(files_data, paths, vflip, mipmaps, errors));
}

} // namespace png2dds::pipeline::impl
