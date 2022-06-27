/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PNG2DDS_MEMORY_HPP
#define PNG2DDS_MEMORY_HPP

#include <cstdint>
#include <span>
#include <vector>

namespace png2dds::memory {

class chunk final {
public:
	explicit chunk(std::size_t size);

	// Chunk is a move-only type.
	chunk(const chunk&) = delete;
	chunk(chunk&&) noexcept = default;
	chunk& operator=(const chunk&) = delete;
	chunk& operator=(chunk&&) noexcept = default;
	~chunk() = default;

	[[nodiscard]] std::span<std::uint8_t> span() noexcept;
	auto operator<=>(const chunk& rhs) const noexcept = default;
	bool operator==(const chunk&) const noexcept = delete;

private:
	std::vector<std::uint8_t> _memory;
};

} // namespace png2dds::memory

#endif // PNG2DDS_MEMORY_HPP
