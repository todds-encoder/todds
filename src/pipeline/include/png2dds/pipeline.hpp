/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "png2dds/input.hpp"

namespace png2dds::pipeline {

/**
 * Encodes a list of PNG files as DDS.
 * @param input_data Input data to use for the pipeline.
 */
void encode_as_dds(const input& input_data);

} // namespace png2dds::pipeline
