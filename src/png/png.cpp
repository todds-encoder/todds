/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/png.hpp"

#include "todds/string.hpp"

#include "spng.h"
#include <fmt/format.h>

#include <cassert>
#include <stdexcept>

namespace {

// RAII wrapper around the spng_ctx object.
class spng_context final {
public:
	explicit spng_context(const todds::string& png, int flags)
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

void set_buffer(spng_context& context, const todds::string& png, std::span<const std::uint8_t> buffer) {
	/* Ignore chunk CRCs and their calculations. */
	spng_set_crc_action(context.get(), SPNG_CRC_USE, SPNG_CRC_USE);

	/* Set memory usage limits for storing standard and unknown chunks. */
	constexpr std::size_t limit = 1024ULL * 1024ULL * 64ULL;
	spng_set_chunk_limits(context.get(), limit, limit);

	if (const int ret = spng_set_png_buffer(context.get(), buffer.data(), buffer.size()); ret != 0) {
		throw std::runtime_error{fmt::format("Could not set PNG file to data {:s}: {:s}", png, spng_strerror(ret))};
	}
}

spng_ihdr get_header(spng_context& context, const todds::string& png) {
	spng_ihdr header{};
	if (const int ret = spng_get_ihdr(context.get(), &header); ret != 0) {
		throw std::runtime_error{fmt::format("Could not read header data of {:s}: {:s}", png, spng_strerror(ret))};
	}
	return header;
}

} // anonymous namespace

namespace todds::png {

std::unique_ptr<mipmap_image> decode(std::size_t file_index, const todds::string& png,
	std::span<const std::uint8_t> buffer, bool flip, std::size_t& width, std::size_t& height, bool mipmaps) {
	width = 0ULL;
	height = 0ULL;
	// Ideally we would want to use SPNG_CTX_IGNORE_ADLER32 here, but unfortunately libspng ignores this value when using
	// miniz.
	spng_context context{png, 0};

	set_buffer(context, png, buffer);
	const spng_ihdr header = get_header(context, png);

	width = header.width;
	height = header.height;
	auto result = std::make_unique<mipmap_image>(file_index, width, height, mipmaps);
	assert(result->mipmap_count() >= 1ULL);
	image& first = result->get_image(0ULL);

	constexpr spng_format format = SPNG_FMT_RGBA8;

	std::size_t file_size{};
	if (const int ret = spng_decoded_image_size(context.get(), format, &file_size); ret != 0) {
		throw std::runtime_error{fmt::format("Could not calculate decoded size of {:s}: {:s}", png, spng_strerror(ret))};
	}

	// The todds data may be larger than the file size because the width and the height must be divisible by 4.
	assert(file_size <= first.data().size());

	if (const int ret = spng_decode_image(context.get(), nullptr, 0, format, SPNG_DECODE_TRNS | SPNG_DECODE_PROGRESSIVE);
			ret != 0) {
		throw std::runtime_error{fmt::format("Could not initialize decoding of {:s}: {:s}", png, spng_strerror(ret))};
	}

	int ret{};
	spng_row_info row_info{};
	const auto file_width = file_size / height;

	do {
		ret = spng_get_row_info(context.get(), &row_info);
		if (ret != 0) { break; }
		const std::size_t row = !flip ? row_info.row_num : height - row_info.row_num - 1UL;
		ret = spng_decode_row(context.get(), &first.row_start(row), file_width);

	} while (ret == 0);

	// Since SPNG_CTX_IGNORE_ADLER32 is not supported for miniz, the SPNG_EIDAT_STREAM raised in this case is ignored.
	if (ret != SPNG_EOI && ret != SPNG_EIDAT_STREAM) {
		throw std::runtime_error{fmt::format("Progressive decode error in {:s}: {:s}", png, spng_strerror(ret))};
	}
	return result;
}

} // namespace todds::png
