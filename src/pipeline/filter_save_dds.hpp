/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <oneapi/tbb/parallel_pipeline.h>

#include "filter_common.hpp"
#include "filter_encode_dds.hpp"

namespace todds::pipeline::impl {

oneapi::tbb::filter<dds_data, void> save_dds_filter(
	const vector<file_data>& files_data, const paths_vector& paths, report_queue& updates);

} // namespace todds::pipeline::impl
