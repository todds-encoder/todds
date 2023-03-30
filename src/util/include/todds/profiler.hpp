/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#if defined(TRACY_ENABLE)
#include <tracy/Tracy.hpp>
#endif

#if defined(TRACY_ENABLE)
#define TracyZoneScopedN(str) ZoneScopedN(str)
#define TracyZoneFileIndex(file_index)                                                                                 \
	const auto file_index_str = std::to_string(file_index);                                                              \
	ZoneText(file_index_str.c_str(), file_index_str.size())

#else
#define TracyZoneScopedN(str)
#define TracyZoneFileIndex(file_index)
#endif
