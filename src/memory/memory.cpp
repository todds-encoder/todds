/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <png2dds/memory.hpp>

namespace png2dds::memory {
std::span<std::uint8_t> chunk::span() const noexcept { return {_memory.get(), _size}; }

chunk reserve::get(std::size_t size) { return chunk{size}; }

void reserve::release(chunk&& /* released_chunk */) {}

} // namespace png2dds::memory
