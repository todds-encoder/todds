/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/input.hpp"
#include "todds/png.hpp"

#include <oneapi/tbb/parallel_pipeline.h>

#include "filter_common.hpp"
#include "filter_encode_png.hpp"

namespace todds::pipeline::impl {

oneapi::tbb::filter<png_data, void> save_png_filter(const paths_vector& paths);

} // namespace todds::pipeline::impl
