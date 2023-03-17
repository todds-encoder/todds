/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "filter_pixel_blocks.hpp"

namespace todds::pipeline::impl {
class get_pixel_blocks final {
public:
	pixel_block_data operator()(const mipmap_image& image) const {
		return pixel_block_data{todds::to_pixel_blocks(image), image.file_index()};
	}
};

oneapi::tbb::filter<mipmap_image, pixel_block_data> load_file_filter() {
	return oneapi::tbb::make_filter<mipmap_image, pixel_block_data>(
		oneapi::tbb::filter_mode::parallel, get_pixel_blocks{});
}

} // namespace todds::pipeline::impl
