/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "filter_generate_mipmaps.hpp"

#include "todds/filter.hpp"
#include "todds/mipmap_image.hpp"

#include <oneapi/tbb/parallel_for.h>
#include <opencv2/imgproc.hpp>

namespace {

void process_image(todds::mipmap_image& mipmap_img, todds::filter::type filter) {
	// constexpr std::uint8_t default_alpha_reference = 245U;
	// const float desired_coverage = todds::alpha_coverage(default_alpha_reference, original_image);
	const auto input = static_cast<cv::Mat>(mipmap_img.get_image(0UL));

	using blocked_range = oneapi::tbb::blocked_range<size_t>;
	oneapi::tbb::parallel_for(oneapi::tbb::blocked_range<size_t>(1UL, mipmap_img.mipmap_count()),
		[&mipmap_img, &input, filter](const blocked_range& range) {
			for (std::size_t mipmap_index = range.begin(); mipmap_index < range.end(); ++mipmap_index) {
				todds::image& current_image = mipmap_img.get_image(mipmap_index);
				auto output = static_cast<cv::Mat>(current_image);
				cv::resize(input, output, output.size(), 0, 0, static_cast<int>(filter));
				// ToDo Solve alpha coverage issues on Windows
				// todds::scale_alpha_to_coverage(desired_coverage, default_alpha_reference, current_image);
			}
		});
}

} // Anonymous namespace

namespace todds::pipeline::impl {

class generate_mipmaps final {
public:
	explicit generate_mipmaps(filter::type filter) noexcept
		: _filter{filter} {}

	std::unique_ptr<mipmap_image> operator()(std::unique_ptr<mipmap_image> img) const {
		process_image(*img, _filter);
		return img;
	}

private:
	filter::type _filter;
};

oneapi::tbb::filter<std::unique_ptr<mipmap_image>, std::unique_ptr<mipmap_image>> generate_mipmaps_filter(
	filter::type filter) {
	return oneapi::tbb::make_filter<std::unique_ptr<mipmap_image>, std::unique_ptr<mipmap_image>>(
		oneapi::tbb::filter_mode::parallel, generate_mipmaps(filter));
}

} // namespace todds::pipeline::impl
