/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "png2dds/input.hpp"
#include "png2dds/vector.hpp"

#include <oneapi/tbb/concurrent_queue.h>
#include <oneapi/tbb/parallel_pipeline.h>

#include <cstdint>

#include "filter_common.hpp"

namespace png2dds::pipeline::impl {

struct png_file {
	vector<std::uint8_t> buffer;
	std::size_t file_index;
};

oneapi::tbb::filter<void, png_file> load_png_filter(
	const paths_vector& paths, std::atomic<std::size_t>& counter, std::atomic<bool>& force_finish, error_queue& errors);

} // namespace png2dds::pipeline::impl
