/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/dds_image.hpp"

#include <dds_defs.h>

namespace {

// dwWidth, dwHeight and dwLinearSize must be filled in later by the caller.
constexpr DDSURFACEDESC2 get_surface_description() noexcept {
	DDSURFACEDESC2 desc{};
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS;

	desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
	desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
	desc.ddpfPixelFormat.dwFlags |= DDPF_FOURCC;

	desc.dwFlags |= DDSD_LINEARSIZE;
	desc.ddpfPixelFormat.dwRGBBitCount = 0;
	desc.ddpfPixelFormat.dwFourCC = PIXEL_FMT_FOURCC('D', 'X', '1', '0'); // NOLINT

	return desc;
}

constexpr DDS_HEADER_DXT10 header_extension{DXGI_FORMAT_BC7_UNORM, D3D10_RESOURCE_DIMENSION_TEXTURE2D, 0U, 1U, 0U};

png2dds::dds_image::header_type get_header(std::size_t width, std::size_t height, std::size_t block_size_bytes) {
	png2dds::dds_image::header_type header;
	// Construct the surface description object directly into the header array memory.
	auto* desc = new (header.data()) DDSURFACEDESC2(get_surface_description());
	desc->dwWidth = static_cast<std::uint32_t>(width);
	desc->dwHeight = static_cast<std::uint32_t>(height);
	desc->dwLinearSize = static_cast<std::uint32_t>(block_size_bytes);
	// Construct the header extension object right after the previous object.
	new (&header[sizeof(DDSURFACEDESC2)]) DDS_HEADER_DXT10(header_extension);
	return header;
}

} // namespace

namespace png2dds {

dds_image::dds_image(const pixel_block_image& image, block_size block_t)
	: _block_offset{static_cast<std::size_t>(block_t)}
	, _width{image.width()}
	, _height{image.height()}
	, _blocks(_width * _height * _block_offset)
	, _header{get_header(image.image_width(), image.image_height(), _blocks.size() * sizeof(std::uint64_t))}
	, _file_index{image.file_index()} {}

const dds_image::header_type& dds_image::header() const noexcept { return _header; }

std::size_t dds_image::width() const noexcept { return _width; }

std::size_t dds_image::height() const noexcept { return _height; }

std::uint64_t* dds_image::block(std::size_t block_x, std::size_t block_y) noexcept {
	return &_blocks[(block_x + block_y * _width) * _block_offset];
}

const dds_image::buffer_type& dds_image::blocks() const noexcept { return _blocks; }

[[nodiscard]] std::size_t dds_image::file_index() const noexcept { return _file_index; }

} // namespace png2dds
