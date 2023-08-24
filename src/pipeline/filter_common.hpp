/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/format.hpp"

#include <oneapi/tbb/concurrent_queue.h>

#include <limits>
#include <string>

namespace todds::pipeline::impl {

using error_queue = oneapi::tbb::concurrent_queue<std::string>;

// Files using this file index have triggered errors and should not be processed.
constexpr std::size_t error_file_index = std::numeric_limits<std::size_t>::max();

struct file_data {
	// Width of the image excluding extra columns. Set during the decoding PNG stage.
	std::size_t width{};
	// Height of the image excluding extra rows. Set during the decoding PNG stage.
	std::size_t height{};
	// Number of mipmap levels in the image, including the main one. Set during the decode PNG stage.
	std::size_t mipmaps{};
	// DDS format of the image. Set during the encoding DDS stage.
	format::type format{};
};

} // namespace todds::pipeline::impl
