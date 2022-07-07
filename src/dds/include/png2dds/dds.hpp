/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PNG2DDS_DDS_HPP
#define PNG2DDS_DDS_HPP

#include "png2dds/image.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ispc {
// Forward-declaration of the internal bc7e object required by the encoder.
struct bc7e_compress_block_params;
} // namespace ispc

namespace png2dds {

class dds_image final {
public:
	using block_type = std::array<std::uint64_t, 2U>;
	using header_type = std::array<char, 144U>;

	explicit dds_image(const image& png);
	dds_image(const dds_image&) = delete;
	dds_image(dds_image&&) = default;
	dds_image& operator=(const dds_image&) = delete;
	dds_image& operator=(dds_image&&) = default;

	~dds_image() = default;

	[[nodiscard]] const header_type& header() const noexcept;
	[[nodiscard]] std::size_t width() const noexcept;
	[[nodiscard]] std::size_t height() const noexcept;
	[[nodiscard]] block_type::value_type* block(std::size_t block_x, std::size_t block_y) noexcept;
	[[nodiscard]] const std::vector<block_type>& blocks() const noexcept;
	[[nodiscard]] std::size_t file_index() const noexcept;

private:
	std::size_t _width;
	std::size_t _height;
	std::vector<block_type> _blocks;
	header_type _header;
	std::size_t _file_index;
};

class encoder final {
public:
	explicit encoder(unsigned int level);
	encoder(const encoder&) = delete;
	encoder(encoder&&) = default;
	encoder& operator=(const encoder&) = delete;
	encoder& operator=(encoder&&) = default;
	~encoder();
	[[nodiscard]] dds_image encode(const image& png) const;

private:
	std::unique_ptr<ispc::bc7e_compress_block_params> _pimpl;
};

} // namespace png2dds

#endif // PNG2DDS_DDS_HPP
