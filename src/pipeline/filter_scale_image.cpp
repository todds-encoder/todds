/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "filter_scale_image.hpp"

#include "todds/filter.hpp"
#include "todds/mipmap_image.hpp"

#include <opencv2/imgproc.hpp>

namespace todds::pipeline::impl {

class scale_image final {
public:
	explicit scale_image(vector<file_data>& files_data, bool mipmaps, std::uint16_t scale, filter::type filter) noexcept
		: _files_data{files_data}
		, _mipmaps{mipmaps}
		, _scale{scale}
		, _filter{filter} {}

	std::unique_ptr<mipmap_image> operator()(std::unique_ptr<mipmap_image> img) const {
		auto& input_image = img->get_image(0U);
		const auto input = static_cast<cv::Mat>(input_image);

		const auto width = (input_image.width() * _scale) / 100U;
		const auto height = (input_image.height() * _scale) / 100U;

		auto result = std::make_unique<mipmap_image>(img->file_index(), width, height, _mipmaps);
		auto output = static_cast<cv::Mat>(result->get_image(0U));
		cv::resize(input, output, output.size(), 0, 0, static_cast<int>(_filter));

		_files_data[result->file_index()].width = width;
		_files_data[result->file_index()].height = height;
		_files_data[result->file_index()].mipmaps = result->mipmap_count();

		return result;
	}

private:
	vector<file_data>& _files_data;
	bool _mipmaps;
	std::uint16_t _scale;
	filter::type _filter;
};

oneapi::tbb::filter<std::unique_ptr<mipmap_image>, std::unique_ptr<mipmap_image>> scale_image_filter(
	vector<file_data>& files_data, bool mipmaps, std::uint16_t scale, filter::type filter) {
	return oneapi::tbb::make_filter<std::unique_ptr<mipmap_image>, std::unique_ptr<mipmap_image>>(
		oneapi::tbb::filter_mode::parallel, scale_image(files_data, mipmaps, scale, filter));
}

} // namespace todds::pipeline::impl
