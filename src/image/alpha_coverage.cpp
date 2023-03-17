/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/alpha_coverage.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

namespace {

constexpr auto bytes_per_pixel = png2dds::image::bytes_per_pixel;

/**
 * Helper function which calculates what the alpha coverage would be if a specific scale was applied.
 * @param alpha_reference Alpha threshold to consider coverage.
 * @param alpha_scale Scaling factor applied to the alpha channel.
 * @param img Image being considered.
 * @return Ratio of pixels above the alpha threshold.
 */
float alpha_coverage_scale(std::uint8_t alpha_reference, float alpha_scale, const png2dds::image& img) {
	std::size_t coverage{};

	for (std::size_t alpha_index = 3UL; alpha_index < img.data().size(); alpha_index += bytes_per_pixel) {
		const auto original_alpha = static_cast<float>(img.data()[alpha_index]);
		const auto scaled_alpha = static_cast<std::uint32_t>(original_alpha * alpha_scale);
		if (scaled_alpha > alpha_reference) { ++coverage; }
	}

	return static_cast<float>(coverage) / static_cast<float>(img.width() * img.height());
}

/**
 * Applies an alpha scaling factor to an image.
 * @param alpha_scale Scaling factor applied to the alpha channel.
 * @param img Image being considered.
 */
void scale_alpha(float alpha_scale, png2dds::image& img) {
	constexpr auto min_alpha = static_cast<std::uint32_t>(std::numeric_limits<std::uint8_t>::min());
	constexpr auto max_alpha = static_cast<std::uint32_t>(std::numeric_limits<std::uint8_t>::max());

	for (std::size_t alpha_index = 3UL; alpha_index < img.data().size(); alpha_index += bytes_per_pixel) {
		const auto original_alpha = static_cast<float>(img.data()[alpha_index]);
		auto scaled_alpha = static_cast<std::uint32_t>(original_alpha * alpha_scale);
		scaled_alpha = std::clamp(min_alpha, scaled_alpha, max_alpha);
		img.data()[alpha_index] = static_cast<std::uint8_t>(scaled_alpha);
	}
}

} // Anonymous namespace

namespace png2dds {

float alpha_coverage(std::uint8_t alpha_reference, const image& img) {
	std::size_t coverage{};

	for (std::size_t alpha_index = 3UL; alpha_index < img.data().size(); alpha_index += image::bytes_per_pixel) {
		if (img.data()[alpha_index] > alpha_reference) { ++coverage; }
	}

	return static_cast<float>(coverage) / static_cast<float>(img.width() * img.height());
}

void scale_alpha_to_coverage(float desired_coverage, std::uint8_t alpha_reference, image& img) {
	constexpr float initial_min_alpha_scale = 0.0F;
	constexpr float initial_max_alpha_scale = 4.0F;
	constexpr std::size_t max_iterations = 10ULL;

	float min_alpha_scale = initial_min_alpha_scale;
	float max_alpha_scale = initial_max_alpha_scale;
	float current_alpha_scale = 1.0F; // The first iteration of the search checks coverage without scaling.
	float best_alpha_scale = std::numeric_limits<float>::max();
	float best_error = std::numeric_limits<float>::max();

	for (std::size_t iteration = 0ULL; iteration < max_iterations; ++iteration) {
		const float current_coverage = alpha_coverage_scale(alpha_reference, current_alpha_scale, img);
		const float current_error = std::fabs(current_coverage - desired_coverage);

		// Update the best candidate.
		if (current_error < best_error) {
			best_error = current_error;
			best_alpha_scale = current_alpha_scale;
		}

		// Update the binary search limits.
		if (current_coverage < desired_coverage) {
			min_alpha_scale = current_alpha_scale;
		} else if (current_coverage > desired_coverage) {
			max_alpha_scale = current_alpha_scale;
		} else {
			break;
		}
		current_alpha_scale = (min_alpha_scale + max_alpha_scale) * 0.5F;
	}

	scale_alpha(best_alpha_scale, img);
}

} // namespace png2dds
