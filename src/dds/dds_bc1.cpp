/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/dds.hpp"
#include "todds/profiler.hpp"

#include <oneapi/tbb/parallel_for.h>

#include "rgbcx_todds.hpp"

namespace {

constexpr std::size_t bc1_block_size = 1UL;

using blocked_range = oneapi::tbb::blocked_range<size_t>;

constexpr std::size_t pixel_block_size = todds::pixel_block_side * todds::pixel_block_side;

} // namespace

namespace todds::dds {

void initialize_bc1_encoding() { rgbcx::init(rgbcx::bc1_approx_mode::cBC1Ideal); }

vector<std::uint64_t> bc1_encode(
	const todds::format::quality quality, const bool alpha_black, const vector<std::uint32_t>& image) {
	constexpr std::size_t grain_size = 64ULL;
	static oneapi::tbb::affinity_partitioner partitioner;
	const std::size_t num_blocks = image.size() / pixel_block_size;

	vector<std::uint64_t> result(num_blocks * bc1_block_size);

	const auto factors = impl::from_quality_level(static_cast<unsigned int>(quality), alpha_black);

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
} // namespace todds::dds
