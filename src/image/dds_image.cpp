/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "png2dds/dds_image.hpp"

#include <dds_defs.h>

#include <cassert>
#include <limits>

namespace {

constexpr std::size_t format_block_size(png2dds::format::type format_type) {
	std::size_t block_size{1UL};
	switch (format_type) {
	case png2dds::format::type::bc1: block_size = 1UL; break;
	case png2dds::format::type::bc7: block_size = 2UL; break;
	case png2dds::format::type::bc1_alpha_bc7: assert(false); break;
	}
	return block_size;
}

constexpr std::uint32_t format_fourcc(png2dds::format::type format_type) {
	std::uint32_t fourcc{};
	switch (format_type) {
	case png2dds::format::type::bc1: fourcc = PIXEL_FMT_FOURCC('D', 'X', 'T', '1'); break;
	case png2dds::format::type::bc7: fourcc = PIXEL_FMT_FOURCC('D', 'X', '1', '0'); break;
	case png2dds::format::type::bc1_alpha_bc7: assert(false); break;
	}
	return fourcc;
}

// dwWidth, dwHeight, dwLinearSize and ddpfPixelFormat.dwFourCC must be filled in later by the caller.
constexpr DDSURFACEDESC2 get_surface_description() noexcept {
	DDSURFACEDESC2 desc{};
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS;

	desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
	desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
	desc.ddpfPixelFormat.dwFlags |= DDPF_FOURCC;

	desc.dwFlags |= DDSD_LINEARSIZE;
	desc.ddpfPixelFormat.dwRGBBitCount = 0;

	return desc;
}

png2dds::dds_image::header_type get_header(
	png2dds::format::type format_type, std::size_t width, std::size_t height, std::size_t block_size_bytes) {
	png2dds::dds_image::header_type header;
	// Construct the surface description object directly into the header array memory.
	auto* desc = new (header.data()) DDSURFACEDESC2(get_surface_description());
	desc->dwWidth = static_cast<std::uint32_t>(width);
	desc->dwHeight = static_cast<std::uint32_t>(height);
	desc->dwLinearSize = static_cast<std::uint32_t>(block_size_bytes);
	desc->ddpfPixelFormat.dwFourCC = format_fourcc(format_type);

	return header;
}

} // namespace

namespace png2dds {

dds_image::dds_image()
	: _block_offset{1U}
	, _width{0UL}
	, _height{0UL}
	, _blocks{}
	, _header{}
	, _file_index{std::numeric_limits<std::size_t>::max()}
	, _format{} {}

dds_image::dds_image(const pixel_block_image& image, format::type format_type)
	: _block_offset{format_block_size(format_type)}
	, _width{image.width()}
	, _height{image.height()}
	, _blocks(_width * _height * _block_offset)
	, _header{get_header(format_type, image.image_width(), image.image_height(), _blocks.size() * sizeof(std::uint64_t))}
	, _file_index{image.file_index()}
	, _format{format_type} {}

const dds_image::header_type& dds_image::header() const noexcept { return _header; }

std::size_t dds_image::width() const noexcept { return _width; }

std::size_t dds_image::height() const noexcept { return _height; }

std::uint64_t* dds_image::block(std::size_t block_x, std::size_t block_y) noexcept {
	return &_blocks[(block_x + block_y * _width) * _block_offset];
}

const dds_image::buffer_type& dds_image::blocks() const noexcept { return _blocks; }

std::size_t dds_image::file_index() const noexcept { return _file_index; }

format::type dds_image::format() const noexcept { return _format; }

} // namespace png2dds
