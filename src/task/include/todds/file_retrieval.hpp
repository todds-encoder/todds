/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/arguments.hpp"
#include "todds/input.hpp"
#include "todds/report.hpp"

namespace todds {

pipeline::paths_vector get_paths(const todds::args::data& arguments, todds::report_queue& updates);

} // namespace todds
