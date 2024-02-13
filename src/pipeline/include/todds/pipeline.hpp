/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/input.hpp"
#include "todds/report.hpp"

namespace todds::pipeline {

/**
 * Encodes a list of PNG files as DDS.
 * @param input_data Input data to use for the pipeline.
 * @param force_finish Used to force the pipeline to finish early, leaving it in a clean state.
 * @param updates The pipeline will report updates back to the caller using this queue.
 */
void encode_as_dds(const input& input_data, std::atomic<bool>& force_finish, todds::report_queue& updates);

} // namespace todds::pipeline
