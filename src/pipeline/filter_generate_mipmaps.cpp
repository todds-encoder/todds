/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "filter_generate_mipmaps.hpp"

#include "todds/filter.hpp"
#include "todds/mipmap_image.hpp"
#include "todds/profiler.hpp"

#include <opencv2/imgproc.hpp>

namespace {

void process_image(todds::mipmap_image& mipmap_img, todds::filter::type filter, double blur) {
	// Used to store images with gaussian blur applied.
	todds::mipmap_image mipmap_blur(mipmap_img);

	for (std::size_t mipmap_index = 1UL; mipmap_index < mipmap_img.mipmap_count(); ++mipmap_index) {
		// Calculate gaussian blur of the previous stage.
		auto& input_current = mipmap_img.get_image(mipmap_index - 1UL);
		auto& blur_current = mipmap_blur.get_image(mipmap_index - 1UL);
		auto blur_mat = static_cast<cv::Mat>(blur_current);
		cv::GaussianBlur(static_cast<cv::Mat>(input_current), blur_mat, {0, 0}, blur, blur);

		// The current mipmap level is calculated by resizing the blurred version of the previous image.
		auto& output_current = mipmap_img.get_image(mipmap_index);
		auto output_mat = static_cast<cv::Mat>(output_current);
		cv::resize(blur_mat, output_mat, output_mat.size(), 0, 0, static_cast<int>(filter));
	}
}

} // Anonymous namespace

namespace todds::pipeline::impl {

class generate_mipmaps final {
public:
	explicit generate_mipmaps(filter::type filter, double blur) noexcept
		: _filter{filter}
		, _blur{blur} {}

	std::unique_ptr<mipmap_image> operator()(std::unique_ptr<mipmap_image> img) const {
		TracyZoneScopedN("mipmap");
		if (img != nullptr) [[likely]] {
			TracyZoneFileIndex(img->file_index());
			process_image(*img, _filter, _blur);
		}
		return img;
	}

private:
	filter::type _filter;
	double _blur;
};

oneapi::tbb::filter<std::unique_ptr<mipmap_image>, std::unique_ptr<mipmap_image>> generate_mipmaps_filter(
	filter::type filter, double blur) {
	return oneapi::tbb::make_filter<std::unique_ptr<mipmap_image>, std::unique_ptr<mipmap_image>>(
		oneapi::tbb::filter_mode::parallel, generate_mipmaps(filter, blur));
}

} // namespace todds::pipeline::impl
