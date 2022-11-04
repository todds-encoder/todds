/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "png2dds/arguments.hpp"

namespace png2dds {

/**
 * Assumes that a UTF-8 locale has been set.
 */
void run(const args::data& arguments);

} // namespace png2dds
