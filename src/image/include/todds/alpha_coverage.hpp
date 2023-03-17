/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/image.hpp"

namespace todds {

/**
 * Obtain the fraction of pixels with an alpha channel value above a threshold.
 * @param alpha_reference Alpha channel threshold.
 * @param img Image being checked.
 * @return Ratio of pixels above the alpha threshold.
 */
float alpha_coverage(std::uint8_t alpha_reference, const image& img);

/**
 * Scales image alpha to keep a desired alpha coverage.
 * @param desired_coverage Coverage ratio to keep.
 * @param alpha_reference Alpha channel threshold.
 * @param img Image to modify.
 */
void scale_alpha_to_coverage(float desired_coverage, std::uint8_t alpha_reference, image& img);

} // namespace todds
