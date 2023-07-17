/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/dds.hpp"

#include "todds/profiler.hpp"
#include "todds/util.hpp"

#include <bc7e_ispc.h>
#include <dds_defs.h>
#include <oneapi/tbb/parallel_for.h>

#include <cassert>

#include "rgbcx_todds.hpp"

namespace {

constexpr std::size_t bc1_block_size = 1UL;

constexpr std::size_t bc7_block_size = 2UL;

using blocked_range = oneapi::tbb::blocked_range<size_t>;

constexpr std::uint32_t format_fourcc(todds::format::type format_type) {
	std::uint32_t fourcc{};
	switch (format_type) {
	case todds::format::type::bc1: fourcc = PIXEL_FMT_FOURCC('D', 'X', 'T', '1'); break;
	case todds::format::type::bc7: fourcc = PIXEL_FMT_FOURCC('D', 'X', '1', '0'); break;
	case todds::format::type::bc1_alpha_bc7: assert(false); break;
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

constexpr std::size_t pixel_block_size = todds::pixel_block_side * todds::pixel_block_side;

} // anonymous namespace

namespace todds::dds {

void initialize_bc1_encoding() { rgbcx::init(rgbcx::bc1_approx_mode::cBC1Ideal); }

vector<std::uint64_t> bc1_encode(
	const todds::format::quality quality, const bool alpha_black, const vector<std::uint32_t>& image) {
	constexpr std::size_t grain_size = 64ULL;
	static oneapi::tbb::affinity_partitioner partitioner;

	vector<std::uint64_t> result(image.size() * bc1_block_size);

	const auto factors = impl::from_quality_level(static_cast<unsigned int>(quality), alpha_black);

	const std::size_t num_blocks = image.size() / pixel_block_size;
	oneapi::tbb::parallel_for(
		blocked_range(0UL, num_blocks, grain_size),
		[factors, &image, &result](const blocked_range& range) {
			TracyZoneScopedN("bc1");
			for (std::size_t block_index = range.begin(); block_index < range.end(); ++block_index) {
				auto* dds_block = &result[block_index * bc1_block_size];
				const auto* pixel_block = reinterpret_cast<const std::uint8_t*>(&image[block_index * pixel_block_size]);
				rgbcx::encode_bc1(dds_block, pixel_block, factors.flags, factors.total_orderings4, factors.total_orderings3);
			}
		},
		partitioner);

	return result;
}

void initialize_bc7_encoding() { ispc::bc7e_compress_block_init(); }

bc7_params bc7_encode_params(todds::format::quality quality) noexcept {
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
	case format::quality::really_slow: {
		// Custom quality level that still provides high quality, while having better performance than slowest.
		ispc::bc7e_compress_block_params_init_slowest(&params, perceptual);
		params.m_opaque_settings.m_max_mode13_partitions_to_try = 4;
		params.m_opaque_settings.m_max_mode0_partitions_to_try = 4;
		params.m_opaque_settings.m_max_mode2_partitions_to_try = 4;
		params.m_alpha_settings.m_max_mode7_partitions_to_try = 4;
		params.m_uber_level = 1;
		break;
	}
	case format::quality::slowest: ispc::bc7e_compress_block_params_init_slowest(&params, perceptual); break;
	}

	return params;
}

vector<std::uint64_t> bc7_encode(const ispc::bc7e_compress_block_params& params, const vector<std::uint32_t>& image) {
	constexpr std::size_t grain_size = 64ULL;
	static oneapi::tbb::affinity_partitioner partitioner;
	const std::size_t num_blocks = image.size() / pixel_block_size;

	vector<std::uint64_t> result(num_blocks * bc7_block_size);

	oneapi::tbb::parallel_for(
		blocked_range(0UL, num_blocks, grain_size),
		[&params, &image, &result](const blocked_range& range) {
			TracyZoneScopedN("bc7");
			const std::size_t block_index = range.begin();
			const auto blocks_to_process = static_cast<std::uint32_t>(range.end() - block_index);
			std::uint64_t* dds_block = &result[block_index * bc7_block_size];
			const auto* pixel_block = &image[block_index * pixel_block_size];
			ispc::bc7e_compress_blocks(static_cast<std::uint32_t>(blocks_to_process), dds_block, pixel_block, &params);
		},
		partitioner);

	return result;
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
