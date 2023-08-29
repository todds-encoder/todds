/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/dds.hpp"
#include "todds/profiler.hpp"

#include <bc7e_ispc.h>
#include <oneapi/tbb/parallel_for.h>

#include "dds_impl.hpp"

namespace {

constexpr std::size_t bc7_block_size = 2UL;

using blocked_range = oneapi::tbb::blocked_range<size_t>;

constexpr std::size_t pixel_block_size = todds::pixel_block_side * todds::pixel_block_side;

} // namespace

namespace todds::dds::impl {
void initialize_bc7_encoding() { ispc::bc7e_compress_block_init(); }
} // namespace todds::dds::impl

namespace todds::dds {

bc7_params bc7_encode_params(todds::format::quality quality) noexcept {
	// Perceptual is currently not supported.
	constexpr bool perceptual = false;
	bc7_params params{};
	switch (quality) {
	case format::quality::ultra_fast: ispc::bc7e_compress_block_params_init_ultrafast(&params, perceptual); break;
	case format::quality::very_fast: ispc::bc7e_compress_block_params_init_veryfast(&params, perceptual); break;
	case format::quality::fast: ispc::bc7e_compress_block_params_init_fast(&params, perceptual); break;
	case format::quality::basic: ispc::bc7e_compress_block_params_init_basic(&params, perceptual); break;
	case format::quality::slow: ispc::bc7e_compress_block_params_init_slow(&params, perceptual); break;
	case format::quality::very_slow: ispc::bc7e_compress_block_params_init_veryslow(&params, perceptual); break;
	case format::quality::really_slow: {
		// Custom quality level that still provides high quality, while having better performance than slowest.
		ispc::bc7e_compress_block_params_init_slowest(&params, perceptual);
		params.m_opaque_settings.m_max_mode13_partitions_to_try = 4;
		params.m_opaque_settings.m_max_mode0_partitions_to_try = 4;
		params.m_opaque_settings.m_max_mode2_partitions_to_try = 4;
		params.m_alpha_settings.m_max_mode7_partitions_to_try = 4;
		params.m_uber_level = 1;
		break;
	}
	case format::quality::slowest: ispc::bc7e_compress_block_params_init_slowest(&params, perceptual); break;
	}

	return params;
}

vector<std::uint64_t> bc7_encode(const ispc::bc7e_compress_block_params& params, const vector<std::uint32_t>& image) {
	constexpr std::size_t grain_size = 64ULL;
	static oneapi::tbb::affinity_partitioner partitioner;
	const std::size_t num_blocks = image.size() / pixel_block_size;

	vector<std::uint64_t> result(num_blocks * bc7_block_size);

	oneapi::tbb::parallel_for(
		blocked_range(0UL, num_blocks, grain_size),
		[&params, &image, &result](const blocked_range& range) {
			TracyZoneScopedN("bc7");
			const std::size_t block_index = range.begin();
			const auto blocks_to_process = static_cast<std::uint32_t>(range.end() - block_index);
			std::uint64_t* dds_block = &result[block_index * bc7_block_size];
			const auto* pixel_block = &image[block_index * pixel_block_size];
			ispc::bc7e_compress_blocks(static_cast<std::uint32_t>(blocks_to_process), dds_block, pixel_block, &params);
		},
		partitioner);

	return result;
}

} // namespace todds::dds
