/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "filter_save_dds.hpp"

#include "png2dds/dds.hpp"

#include <boost/nowide/fstream.hpp>
#include <dds_defs.h>

#include "filter_pixel_blocks.hpp"

namespace png2dds::pipeline::impl {

// Header extension for BC7 files.
constexpr DDS_HEADER_DXT10 header_extension{DXGI_FORMAT_BC7_UNORM, D3D10_RESOURCE_DIMENSION_TEXTURE2D, 0U, 1U, 0U};

class save_dds_file final {
public:
	explicit save_dds_file(const std::vector<file_data>& files_data, const paths_vector& paths) noexcept
		: _files_data{files_data}
		, _paths{paths} {}

	void operator()(const dds_data& dds_img) const {
		const std::size_t file_index = dds_img.file_index;
		if (dds_img.file_index == error_file_index) [[unlikely]] { return; }

#if BOOST_OS_WINDOWS
		const boost::filesystem::path output{R"(\\?\)" + _paths[file_index].second.string()};
#else
		const boost::filesystem::path& output{_paths[file_index].second.string()};
#endif

		boost::nowide::ofstream ofs{output, std::ios::out | std::ios::binary};

		ofs << "DDS ";
		const std::size_t block_size_bytes = dds_img.image.size() * sizeof(std::uint64_t);
		const auto& file_data = _files_data[file_index];
		const auto header = png2dds::dds::dds_header(
			file_data.format, file_data.width, file_data.height, block_size_bytes, file_data.mipmaps);
		ofs.write(header.data(), header.size());
		if (file_data.format == png2dds::format::type::bc7) {
			ofs.write(reinterpret_cast<const char*>(&header_extension), sizeof(header_extension));
		}

		ofs.write(reinterpret_cast<const char*>(dds_img.image.data()), static_cast<std::ptrdiff_t>(block_size_bytes));
		ofs.close();
	}

private:
	const std::vector<file_data>& _files_data;
	const paths_vector& _paths;
};

oneapi::tbb::filter<dds_data, void> save_dds_filter(
	const std::vector<file_data>& files_data, const paths_vector& paths) {
	return oneapi::tbb::make_filter<dds_data, void>(oneapi::tbb::filter_mode::parallel, save_dds_file(files_data, paths));
}

} // namespace png2dds::pipeline::impl