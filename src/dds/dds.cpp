/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/dds.hpp"

#include <bc7e_ispc.h>
#include <dds_defs.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <type_traits>
#include <vector>

namespace {

// Pixel blocks are squares of size block_side * block_side.
constexpr std::size_t pixel_block_side = 4UL;

// Number of bytes in a pixel block.
constexpr std::size_t pixel_block_byte_size = pixel_block_side * pixel_block_side * png2dds::image::bytes_per_pixel;

// Number of blocks to process on each encoding pass.
constexpr std::size_t blocks_to_process = 64UL;

// Temporary storage of pixel blocks stored as contiguous data. Used as the input for encoding.
using pixel_block_array = std::array<std::uint8_t, blocks_to_process * pixel_block_byte_size>;

void get_pixel_blocks(const png2dds::image& png, std::size_t num_blocks, pixel_block_array& pixel_blocks,
	std::size_t pixel_block_x, std::size_t pixel_block_y) {
	assert(pixel_blocks.size() > num_blocks);
	// This function only processes blocks in the same row. The starting vertical coordinate is constant.
	const auto pixel_y = pixel_block_y * pixel_block_side;
	assert(pixel_y < png.padded_height());
	// Blocks will be copied sequentially in the output array.
	auto* pixel_block_itr = pixel_blocks.begin();

	for (std::size_t block_x_offset = 0UL; block_x_offset < num_blocks; ++block_x_offset) {
		// The starting x coordinate must be adjusted for each block to copy.
		const auto pixel_x = (pixel_block_x + block_x_offset) * pixel_block_side;
		assert(pixel_x < png.padded_width());
		for (std::size_t offset_y = 0U; offset_y < pixel_block_side; ++offset_y) {
			const std::uint8_t* pixel_start = png.get_pixel(pixel_x, pixel_y + offset_y).data();
			assert(pixel_start < &png.buffer().back());
			const std::uint8_t* pixel_end = png.get_pixel(pixel_x + pixel_block_side, pixel_y + offset_y).data();
			assert(pixel_end <= &*png.buffer().end());
			assert(std::distance(pixel_start, pixel_end) == pixel_block_side * png2dds::image::bytes_per_pixel);
			pixel_block_itr = std::copy(pixel_start, pixel_end, pixel_block_itr);
			assert(pixel_block_itr <= pixel_blocks.end());
		}
	}
}

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

} // anonymous namespace

namespace png2dds {

// The header must have exactly enough memory to store all data from these two types.
static_assert(sizeof(DDSURFACEDESC2) + sizeof(DDS_HEADER_DXT10) == sizeof(dds_image::header_type));

// Blocks must meet these constraints.
static_assert(std::is_same_v<dds_image::block_type::value_type, std::uint64_t>);
static_assert(sizeof(png2dds::dds_image::block_type) == 16U);

dds_image::dds_image(const image& png)
	: _width{png.padded_width() / pixel_block_side}
	, _height{png.padded_height() / pixel_block_side}
	, _blocks(_width * _height)
	, _header{get_header(png.width(), png.height(), _blocks.size() * sizeof(dds_image::block_type))} {}

const dds_image::header_type& dds_image::header() const noexcept { return _header; }

std::size_t dds_image::width() const noexcept { return _width; }

std::size_t dds_image::height() const noexcept { return _height; }

dds_image::block_type::value_type* dds_image::block(std::size_t block_x, std::size_t block_y) noexcept {
	return _blocks[block_x + block_y * _width].data();
}

std::vector<dds_image::block_type>& dds_image::blocks() noexcept { return _blocks; }

const std::vector<dds_image::block_type>& dds_image::blocks() const noexcept { return _blocks; }

encoder::encoder(unsigned int level)
	: _pimpl(std::make_unique<ispc::bc7e_compress_block_params>()) {
	// Encoder initialization.
	ispc::bc7e_compress_block_init();
	// Encoder parameter initialization. Perceptual is currently not supported.
	constexpr bool perceptual = false;
	switch (level) {
	case 0U: ispc::bc7e_compress_block_params_init_ultrafast(_pimpl.get(), perceptual); break;
	case 1U: ispc::bc7e_compress_block_params_init_veryfast(_pimpl.get(), perceptual); break;
	case 2U: ispc::bc7e_compress_block_params_init_fast(_pimpl.get(), perceptual); break;
	case 3U: ispc::bc7e_compress_block_params_init_basic(_pimpl.get(), perceptual); break;
	case 4U: ispc::bc7e_compress_block_params_init_slow(_pimpl.get(), perceptual); break;
	case 5U: ispc::bc7e_compress_block_params_init_veryslow(_pimpl.get(), perceptual); break;
	default:
	case 6U: ispc::bc7e_compress_block_params_init_slowest(_pimpl.get(), perceptual); break;
	}
}

encoder::~encoder() = default;

dds_image encoder::encode(const image& png) const {
	assert((png.padded_width() % pixel_block_side) == 0);
	assert((png.padded_height() % pixel_block_side) == 0);

	dds_image dds_img(png);

	// Stores squares of pixel blocks as contiguous data, to serve as the encoding input.
	std::array<std::uint8_t, blocks_to_process * pixel_block_byte_size> pixel_blocks{};

	for (std::size_t block_y = 0UL; block_y < dds_img.height(); ++block_y) {
		for (std::size_t block_x = 0UL; block_x < dds_img.width(); block_x += blocks_to_process) {
			// In some cases the number of blocks of the image may not be divisible by 64.
			const std::size_t current_blocks_to_process = std::min(blocks_to_process, dds_img.width() - block_x);
			// Copy input data as pixel blocks.
			get_pixel_blocks(png, current_blocks_to_process, pixel_blocks, block_x, block_y);

			auto* dds_block = dds_img.block(block_x, block_y);
			ispc::bc7e_compress_blocks(static_cast<std::uint32_t>(current_blocks_to_process), dds_block,
				reinterpret_cast<const std::uint32_t*>(pixel_blocks.data()), _pimpl.get());
		}
	}

	return dds_img;
}

} // namespace png2dds
