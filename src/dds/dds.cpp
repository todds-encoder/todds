/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/dds.hpp"

#include "todds/util.hpp"

#include <dds_defs.h>

#include <cassert>

#include "dds_impl.hpp"

namespace {

constexpr std::uint32_t format_fourcc(todds::format::type format_type) {
	std::uint32_t fourcc{};
	switch (format_type) {
	case todds::format::type::bc1: fourcc = PIXEL_FMT_FOURCC('D', 'X', 'T', '1'); break;
	case todds::format::type::bc7: fourcc = PIXEL_FMT_FOURCC('D', 'X', '1', '0'); break;
	case todds::format::type::invalid: assert(false); break;
	}
	return fourcc;
}

// dwWidth, dwHeight, dwLinearSize and ddpfPixelFormat.dwFourCC must be filled in later by the caller.
// dwFlags and dwCaps may need to be modified.
constexpr DDSURFACEDESC2 get_surface_description() noexcept {
	DDSURFACEDESC2 desc{};
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS | DDSD_LINEARSIZE;
	desc.dwBackBufferCount = 1UL;

	desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
	desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
	desc.ddpfPixelFormat.dwFlags |= DDPF_FOURCC;

	desc.ddpfPixelFormat.dwRGBBitCount = 0;

	return desc;
}

} // anonymous namespace

namespace todds::dds {

void initialize_encoding(format::type format, format::type alpha_format) {
	if (format == format::type::bc1 || alpha_format == format::type::bc1) { impl::initialize_bc1_encoding(); }
	if (format == format::type::bc7 || alpha_format == format::type::bc7) { impl::initialize_bc7_encoding(); }
}

std::array<char, 124> dds_header(
	todds::format::type format_type, std::size_t width, std::size_t height, std::size_t mipmaps) {
	std::array<char, 124> header{};
	// Construct the surface description object directly into the header array memory.
	auto* desc = new (header.data()) DDSURFACEDESC2(get_surface_description());
	// Modify any values specific to this image.
	desc->dwWidth = static_cast<std::uint32_t>(width);
	desc->dwHeight = static_cast<std::uint32_t>(height);
	desc->lPitch = static_cast<std::int32_t>(util::next_divisible_by_4(width) * util::next_divisible_by_4(height));
	if (mipmaps > 0) {
		desc->dwFlags |= DDSD_MIPMAPCOUNT;
		desc->dwMipMapCount = static_cast<std::uint32_t>(mipmaps);
		desc->ddsCaps.dwCaps |= DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
	}
	desc->ddpfPixelFormat.dwFourCC = format_fourcc(format_type);

	return header;
}

} // namespace todds::dds
