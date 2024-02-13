/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/arguments.hpp"
#include "todds/report.hpp"

#include <atomic>
#include <future>

namespace todds {

/**
 * Launches the todds encoding pipeline with a specific set of arguments.
 * @param arguments Input data and options to use for encoding.
 * @param force_finish Used to force the pipeline to finish early, leaving it in a clean state.
 * @param updates The pipeline will report updates back to the caller using this queue.
 * @return Result of the parallel pipeline execution.
 */
std::future<void> run(const args::data& arguments, std::atomic<bool>& force_finish, report_queue& updates);

} // namespace todds
