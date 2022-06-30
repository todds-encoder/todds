/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/png.hpp"

#include <fmt/format.h>

#include <stdexcept>

#include "spng.h"

namespace {

// RAII wrapper around the spng_ctx object.
class spng_context final {
public:
	static constexpr std::size_t limit = 1024UL * 1024UL * 64UL;

	explicit spng_context(const std::string& png)
		: _ctx{spng_ctx_new(0)} {
		if (_ctx == nullptr) { throw std::runtime_error{fmt::format("libspng context creation failed for {:s}", png)}; }

		/* Ignore chunk CRCs and their calculations. */
		spng_set_crc_action(_ctx, SPNG_CRC_USE, SPNG_CRC_USE);

		/* Set memory usage limits for storing standard and unknown chunks. */
		spng_set_chunk_limits(_ctx, limit, limit);
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

namespace png2dds {

image decode(const std::string& png, const std::vector<std::uint8_t>& buffer) {
	spng_context context{png};

	if (const int ret = spng_set_png_buffer(context.get(), buffer.data(), buffer.size()); ret != 0) {
		throw std::runtime_error{fmt::format("Could not set PNG file to buffer {:s}: {:s}", png, spng_strerror(ret))};
	}

	spng_ihdr header{};
	if (const int ret = spng_get_ihdr(context.get(), &header); ret != 0) {
		throw std::runtime_error{fmt::format("Could not read header data of {:s}: {:s}", png, spng_strerror(ret))};
	}

	image result(header.width, header.height);
	constexpr spng_format format = SPNG_FMT_RGBA8;
	if (const int ret = spng_decode_image(context.get(), result.buffer(), result.size(), format, 0); ret != 0) {
		throw std::runtime_error{fmt::format("Error during decoding of {:s}: {:s}", png, spng_strerror(ret))};
	}

	return result;
}

} // namespace png2dds
