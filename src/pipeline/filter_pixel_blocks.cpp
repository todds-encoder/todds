/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "filter_pixel_blocks.hpp"

#include "todds/profiler.hpp"

#if defined(TODDS_PIPELINE_DUMP)
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/nowide/fstream.hpp>
#endif // defined(TODDS_PIPELINE_DUMP)

namespace todds::pipeline::impl {
class get_pixel_blocks final {
public:
	pixel_block_data operator()(std::unique_ptr<mipmap_image> image) const {
		TracyZoneScopedN("blocks");
		if (image == nullptr) [[unlikely]] { return {{}, error_file_index}; }
		TracyZoneFileIndex(image->file_index());

		pixel_block_data data{to_pixel_blocks(*image), image->file_index()};
#if defined(TODDS_PIPELINE_DUMP)
		const auto dmp_path = boost::dll::program_location().parent_path() / "pixel_blocks.dmp";
		boost::nowide::ofstream dmp{dmp_path, std::ios::out | std::ios::binary};
		const std::uint32_t* image_start = data.image.data();
		dmp.write(reinterpret_cast<const char*>(image_start),
			static_cast<std::ptrdiff_t>(data.image.size() * sizeof(std::uint32_t)));
#endif // defined(TODDS_PIPELINE_DUMP)
		return data;
	}
};

oneapi::tbb::filter<std::unique_ptr<mipmap_image>, pixel_block_data> pixel_blocks_filter() {
	return oneapi::tbb::make_filter<std::unique_ptr<mipmap_image>, pixel_block_data>(
		oneapi::tbb::filter_mode::parallel, get_pixel_blocks{});
}

} // namespace todds::pipeline::impl
