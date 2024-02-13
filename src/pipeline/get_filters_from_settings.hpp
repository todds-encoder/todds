/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "todds/input.hpp"

#include <oneapi/tbb/parallel_pipeline.h>

#include "filter_common.hpp"

namespace todds::pipeline::impl {

oneapi::tbb::filter<void, void> get_filters_from_settings(const input& input_data, std::atomic<std::size_t>& counter,
	std::atomic<bool>& force_finish, report_queue& error_log, vector<impl::file_data>& files_data);

} // namespace todds::pipeline::impl
