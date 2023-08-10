#pragma once

#include <cassert>
#include <cstdint>

#include "rgbcx.h"

namespace todds::dds::impl {

struct factor_values {
	std::uint32_t flags{};
	std::uint32_t total_orderings4{1};
	std::uint32_t total_orderings3{1};
};

constexpr std::uint32_t quality_flags(const std::uint32_t level) {
	using namespace rgbcx; // NOLINT
	switch (level) {
	case 1U: return cEncodeBC1TwoLeastSquaresPasses;
	case 2U: return cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseFasterMSEEval | cEncodeBC1UseLikelyTotalOrderings;
	case 3U:
	case 4U: return cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseLikelyTotalOrderings;
	case 5U:
		return cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseFullMSEEval | cEncodeBC1UseLikelyTotalOrderings |
					 cEncodeBC1Use6PowerIters | (32U << cEncodeBC1EndpointSearchRoundsShift) | cEncodeBC1TryAllInitialEndponts;
	default:
	case 6U:
	case 7U:
		return cEncodeBC1TwoLeastSquaresPasses | cEncodeBC1UseFullMSEEval | cEncodeBC1UseLikelyTotalOrderings |
					 cEncodeBC1Use6PowerIters | cEncodeBC1Iterative | (256U << cEncodeBC1EndpointSearchRoundsShift) |
					 cEncodeBC1TryAllInitialEndponts;
	}
}

constexpr std::uint32_t quality_total_orderings4(const std::uint32_t level) {
	switch (level) {
	case 1U:
	case 2U: return 1U;
	case 3U: return 11U;
	case 4U: return 32U;
	case 5U: return ((((32 + rgbcx::MAX_TOTAL_ORDERINGS4) / 2) + 32) / 2);
	default:
	case 6U:
	case 7U: return rgbcx::MAX_TOTAL_ORDERINGS4;
	}
}

constexpr std::uint32_t quality_total_orderings3(const std::uint32_t level) {
	switch (level) {
	case 1U:
	case 2U: return 1U;
	case 3U: return 3U;
	default:
	case 4U:
	case 5U:
	case 6U:
	case 7U: return 32U;
	}
}

/**
 * Calculate the BC1 encoding factors to be used by encode_bc1.
 * @param level Quality level set by todds. Levels 6 and 7 are equivalent for BC1.
 * @param alpha_black Use 3 color blocks for blocks containing black or very dark pixels. Increases texture quality
 * substantially, but programs using these textures must ignore the alpha channel.
 * @return Factors to use for these input parameters.
 */
static factor_values from_quality_level(std::uint32_t level, const bool alpha_black) {
	factor_values factors{};
	factors.flags = quality_flags(level);
	factors.total_orderings4 = quality_total_orderings4(level);
	factors.total_orderings3 = quality_total_orderings3(level);

	if (level > 1U) {
		// The encoder can use 3-color mode for a small but noticeable gain in average quality, but lower performance.
		constexpr bool allow_3color = true;
		if (allow_3color) { factors.flags |= rgbcx::cEncodeBC1Use3ColorBlocks; }
		if (alpha_black) { factors.flags |= rgbcx::cEncodeBC1Use3ColorBlocksForBlackPixels; }
	}

	return factors;
}

} // namespace todds::dds::impl
