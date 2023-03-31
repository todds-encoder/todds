#pragma once

#include <cassert>
#include <cstdint>

#include "rgbcx.h"

namespace rgbcx_todds {

// A necessary evil in this case.
using namespace rgbcx; // NOLINT

struct factor_values {
	std::uint32_t flags{};
	std::uint32_t total_orderings4{1};
	std::uint32_t total_orderings3{1};
};

// encode_bc1(uint32_t level... calculates multiple factors depending on current quality level for each block.
// This function calculates the same factors, allowing to calculate them only once.
// Assumes that allow_3color and allow_transparent_texels_for_black are always true.
// Expects levels in the [0, 7] range, and maps them to rgbcx's [0-18] range.
static factor_values from_quality_level(std::uint32_t level) {
	std::uint32_t flags = 0;
	std::uint32_t total_orderings4 = 1;
	std::uint32_t total_orderings3 = 1;
	constexpr bool allow_3color = true;
	constexpr bool allow_transparent_texels_for_black = true;

	switch (level) {
	case 1U:
		// Slightly stronger than stb_dxt HIGHQUAL.
		flags = cEncodeBC1TwoLeastSquaresPasses;
		break;
	case 2U:
		flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseFasterMSEEval | cEncodeBC1UseLikelyTotalOrderings;
		flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) |
						 (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
		break;
	case 3U:
		flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseLikelyTotalOrderings;
		flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) |
						 (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
		total_orderings4 = 11;
		total_orderings3 = 3;
		break;
	case 4U:
		flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseLikelyTotalOrderings;
		flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) |
						 (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
		total_orderings4 = 32;
		total_orderings3 = 32;
		break;
	case 5U:
		flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseFullMSEEval | cEncodeBC1UseLikelyTotalOrderings |
						cEncodeBC1Use6PowerIters | (32U << cEncodeBC1EndpointSearchRoundsShift) | cEncodeBC1TryAllInitialEndponts;
		flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) |
						 (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
		total_orderings4 = ((((32 + MAX_TOTAL_ORDERINGS4) / 2) + 32) / 2);
		total_orderings3 = 32;
		break;
	case 6U:
	case 7U:
		flags = cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseFullMSEEval | cEncodeBC1UseLikelyTotalOrderings |
						cEncodeBC1Use6PowerIters | cEncodeBC1Iterative | (256U << cEncodeBC1EndpointSearchRoundsShift) |
						cEncodeBC1TryAllInitialEndponts;
		flags |= (allow_3color ? cEncodeBC1Use3ColorBlocks : 0) |
						 (allow_transparent_texels_for_black ? cEncodeBC1Use3ColorBlocksForBlackPixels : 0);
		total_orderings4 = MAX_TOTAL_ORDERINGS4;
		total_orderings3 = 32;
		break;
	default: assert(false); break;
	}

	factor_values new_factors;
	new_factors.flags = flags;
	new_factors.total_orderings4 = total_orderings4;
	new_factors.total_orderings3 = total_orderings3;

	return new_factors;
}

} // namespace rgbcx_todds
