/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "filter_save_png.hpp"

#include "todds/profiler.hpp"

#include <boost/nowide/fstream.hpp>
#include <boost/predef.h>

#include "filter_pixel_blocks.hpp"

namespace todds::pipeline::impl {

class save_png_file final {
public:
	explicit save_png_file(const paths_vector& paths) noexcept
		: _paths{paths} {}

	void operator()(const png_data& input) const {
		TracyZoneScopedN("save_png");
		const std::size_t file_index = input.file_index;
		if (file_index == error_file_index) [[unlikely]] { return; }
		TracyZoneFileIndex(file_index);

#if BOOST_OS_WINDOWS
		const boost::filesystem::path output_path{R"(\\?\)" + _paths[file_index].second.string()};
#else
		const boost::filesystem::path& output_path{_paths[file_index].second.string()};
#endif

		boost::nowide::ofstream ofs{output_path, std::ios::out | std::ios::binary};

		const auto size = static_cast<std::ptrdiff_t>(input.image.size());
		ofs.write(reinterpret_cast<const char*>(input.image.data()), size);
		ofs.close();
	}

private:
	const paths_vector& _paths;
};

oneapi::tbb::filter<png_data, void> save_png_filter(const paths_vector& paths) {
	return oneapi::tbb::make_filter<png_data, void>(oneapi::tbb::filter_mode::parallel, save_png_file(paths));
}

} // namespace todds::pipeline::impl
