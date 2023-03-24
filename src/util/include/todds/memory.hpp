/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

#if defined(TODDS_TBB_ALLOCATOR)
#include <oneapi/tbb/scalable_allocator.h>
#endif

namespace todds {

/** Allocator to use in todds types. */
template<typename Type>
using allocator =
#if defined(TODDS_TBB_ALLOCATOR)
	tbb::scalable_allocator<Type>
#else
	std::allocator<Type>
#endif //
	;

} // namespace todds
