/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/dds.hpp"

#include <bc7e_ispc.h>
#include <dds_defs.h>
#include <oneapi/tbb/parallel_for.h>
#include <rgbcx_png2dds.h>

#include <cassert>

namespace {

// Number of blocks to process on each BC7 encoding pass.
constexpr std::size_t blocks_to_process = 64UL;

constexpr std::size_t bc1_block_size = 1UL;

constexpr std::size_t bc7_block_size = 2UL;

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
// dwFlags and dwCaps may need to be modified.
constexpr DDSURFACEDESC2 get_surface_description() noexcept {
	DDSURFACEDESC2 desc{};
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS | DDSD_LINEARSIZE;

	desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
	desc.ddpfPixelFormat.dwSize = sizeof(desc.ddpfPixelFormat);
	desc.ddpfPixelFormat.dwFlags |= DDPF_FOURCC;

	desc.ddpfPixelFormat.dwRGBBitCount = 0;

	return desc;
}

constexpr std::size_t pixel_block_size = png2dds::pixel_block_side * png2dds::pixel_block_side;

} // anonymous namespace

namespace png2dds::dds {

void initialize_bc1_encoding() { rgbcx::init(rgbcx::bc1_approx_mode::cBC1Ideal); }

vector<std::uint64_t> bc1_encode(png2dds::format::quality quality, const vector<std::uint32_t>& image) {
	vector<std::uint64_t> result(image.size() * bc1_block_size);

	// Since it allows quality between [0, 18], multiply png2dds's [0, 6] range by 3.
	const auto factors = rgbcx_png2dds::from_quality_level(static_cast<unsigned int>(quality) * 3U);

	const std::size_t num_blocks = image.size() / pixel_block_size;
	using blocked_range = oneapi::tbb::blocked_range<size_t>;
	oneapi::tbb::parallel_for(blocked_range(0UL, num_blocks), [factors, &image, &result](const blocked_range& range) {
		for (std::size_t block_index = range.begin(); block_index < range.end(); ++block_index) {
			auto* dds_block = &result[block_index * bc1_block_size];
			const auto* pixel_block = reinterpret_cast<const std::uint8_t*>(&image[block_index * pixel_block_size]);
			rgbcx::encode_bc1(dds_block, pixel_block, factors.flags, factors.total_orderings4, factors.total_orderings3);
		}
	});

	return result;
}

void initialize_bc7_encoding() { ispc::bc7e_compress_block_init(); }

bc7_params bc7_encode_params(png2dds::format::quality quality) noexcept {
	// Perceptual is currently not supported.
	constexpr bool perceptual = false;
	bc7_params params{};
	switch (quality) {
	case format::quality::ultra_fast: ispc::bc7e_compress_block_params_init_ultrafast(&params, perceptual); break;
	case format::quality::very_fast: ispc::bc7e_compress_block_params_init_veryfast(&params, perceptual); break;
	case format::quality::fast: ispc::bc7e_compress_block_params_init_fast(&params, perceptual); break;
	case format::quality::basic: ispc::bc7e_compress_block_params_init_basic(&params, perceptual); break;
	case format::quality::slow: ispc::bc7e_compress_block_params_init_slow(&params, perceptual); break;
	case format::quality::very_slow: ispc::bc7e_compress_block_params_init_veryslow(&params, perceptual); break;
	case format::quality::slowest: ispc::bc7e_compress_block_params_init_slowest(&params, perceptual); break;
	}

	return params;
}

vector<std::uint64_t> bc7_encode(const ispc::bc7e_compress_block_params& params, const vector<std::uint32_t>& image) {
	vector<std::uint64_t> result(image.size() * bc7_block_size, 0UL);

	const std::size_t num_blocks = image.size() / pixel_block_size;

	using blocked_range = oneapi::tbb::blocked_range<size_t>;
	oneapi::tbb::parallel_for(blocked_range(0UL, num_blocks), [&params, &image, &result](const blocked_range& range) {
		std::size_t block_index = range.begin();

		while (block_index < range.end()) {
			const std::size_t current_blocks_to_process = std::min(blocks_to_process, range.end() - block_index);

			std::uint64_t* dds_block = &result[block_index * bc7_block_size];
			const auto* pixel_block = &image[block_index * pixel_block_size];
			ispc::bc7e_compress_blocks(
				static_cast<std::uint32_t>(current_blocks_to_process), dds_block, pixel_block, &params);
			block_index += current_blocks_to_process;
		}
	});

	return result;
}

std::array<char, 124> dds_header(png2dds::format::type format_type, std::size_t width, std::size_t height,
	std::size_t block_size_bytes, std::size_t mipmaps) {
	std::array<char, 124> header{};
	// Construct the surface description object directly into the header array memory.
	auto* desc = new (header.data()) DDSURFACEDESC2(get_surface_description());
	// Modify any values specific to this image.
	desc->dwWidth = static_cast<std::uint32_t>(width);
	desc->dwHeight = static_cast<std::uint32_t>(height);
	desc->dwLinearSize = static_cast<std::uint32_t>(block_size_bytes);
	if (mipmaps > 0) {
		desc->dwFlags |= DDSD_MIPMAPCOUNT;
		desc->dwMipMapCount = static_cast<std::uint32_t>(mipmaps);
		desc->ddsCaps.dwCaps |= DDSCAPS_COMPLEX | DDSCAPS_MIPMAP;
	}
	desc->ddpfPixelFormat.dwFourCC = format_fourcc(format_type);

	return header;
}

} // namespace png2dds::dds
