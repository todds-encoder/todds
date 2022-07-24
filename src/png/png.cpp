/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/png.hpp"

#include "spng.h"
#include <fmt/format.h>

#include <cstddef>
#include <stdexcept>

namespace {

// RAII wrapper around the spng_ctx object.
class spng_context final {
public:
	explicit spng_context(const std::string& png, int flags)
		: _ctx{spng_ctx_new(flags)} {
		if (_ctx == nullptr) { throw std::runtime_error{fmt::format("libspng context creation failed for {:s}", png)}; }
	}

	spng_context(const spng_context&) = delete;
	spng_context(spng_context&&) noexcept = delete;
	spng_context& operator=(const spng_context&) = delete;
	spng_context& operator=(spng_context&&) noexcept = delete;

	~spng_context() { spng_ctx_free(_ctx); }

	spng_ctx* get() { return _ctx; }

private:
	spng_ctx* _ctx;
};

} // anonymous namespace

namespace png2dds::png {

image decode(std::size_t file_index, const std::string& png, std::span<const std::uint8_t> buffer, bool flip) {
	spng_context context{png, 0};

	/* Ignore chunk CRCs and their calculations. */
	spng_set_crc_action(context.get(), SPNG_CRC_USE, SPNG_CRC_USE);

	/* Set memory usage limits for storing standard and unknown chunks. */
	constexpr std::size_t limit = 1024UL * 1024UL * 64UL;
	spng_set_chunk_limits(context.get(), limit, limit);

	if (const int ret = spng_set_png_buffer(context.get(), buffer.data(), buffer.size()); ret != 0) {
		throw std::runtime_error{fmt::format("Could not set PNG file to buffer {:s}: {:s}", png, spng_strerror(ret))};
	}

	spng_ihdr header{};
	if (const int ret = spng_get_ihdr(context.get(), &header); ret != 0) {
		throw std::runtime_error{fmt::format("Could not read header data of {:s}: {:s}", png, spng_strerror(ret))};
	}

	image result(file_index, header.width, header.height);

	constexpr spng_format format = SPNG_FMT_RGBA8;
	// The png2dds buffer may be larger than the image size calculated by libspng because the buffer must ensure that
	// the pixels of the width and the row are divisible by 4.
	std::size_t file_size{};
	if (const int ret = spng_decoded_image_size(context.get(), format, &file_size); ret != 0) {
		throw std::runtime_error{fmt::format("Could not calculate decoded size of {:s}: {:s}", png, spng_strerror(ret))};
	}

	if (file_size > result.buffer().size()) {
		throw std::runtime_error{
			fmt::format("Could not fit {:s} into the buffer. Expected size: {:d}, calculated size: {:d}", png,
				result.buffer().size(), file_size)};
	}

	if (const int ret = spng_decode_image(context.get(), nullptr, 0, format, SPNG_DECODE_TRNS | SPNG_DECODE_PROGRESSIVE);
			ret != 0) {
		throw std::runtime_error{fmt::format("Could not initialize decoding of {:s}: {:s}", png, spng_strerror(ret))};
	}

	int ret{};
	spng_row_info row_info{};
	const auto file_width = file_size / header.height;

	do {
		ret = spng_get_row_info(context.get(), &row_info);
		if (ret != 0) { break; }
		const std::size_t row = !flip ? row_info.row_num : result.height() - row_info.row_num - 1UL;
		ret = spng_decode_row(context.get(), &result.get_byte(0UL, row), file_width);

	} while (ret == 0);

	if (ret != SPNG_EOI) {
		throw std::runtime_error{fmt::format("Progressive decode error in {:s}: {:s}", png, spng_strerror(ret))};
	}

	// When padding has been added to the image, copy the border pixel into the padding.
	if (result.width() < result.padded_width()) {
		const auto border_pixel_x = result.width() - 1UL;
		for (std::size_t pixel_y = 0UL; pixel_y < result.padded_height(); ++pixel_y) {
			const auto border_pixel = result.get_pixel(border_pixel_x, std::min(pixel_y, result.height() - 1U));
			for (std::size_t pixel_x = result.width(); pixel_x < result.padded_width(); ++pixel_x) {
				const auto current_pixel = result.get_pixel(pixel_x, pixel_y);
				std::copy(border_pixel.begin(), border_pixel.end(), current_pixel.begin());
			}
		}
	}
	if (result.height() < result.padded_height()) {
		const auto border_pixel_y = result.height() - 1UL;
		for (std::size_t pixel_x = 0UL; pixel_x < result.padded_width(); ++pixel_x) {
			const auto border_pixel = result.get_pixel(std::min(pixel_x, result.width() - 1U), border_pixel_y);
			for (std::size_t pixel_y = result.height(); pixel_y < result.padded_height(); ++pixel_y) {
				const auto current_pixel = result.get_pixel(pixel_x, pixel_y);
				std::copy(border_pixel.begin(), border_pixel.end(), current_pixel.begin());
			}
		}
	}

	return result;
}

} // namespace png2dds::png
