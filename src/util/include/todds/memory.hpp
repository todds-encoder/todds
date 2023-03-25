/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>

#if defined(TODDS_TBB_ALLOCATOR)
#include <oneapi/tbb/scalable_allocator.h>
#elif defined(TODDS_MIMALLOC_ALLOCATOR)
#include <mimalloc.h>
#endif

namespace todds {

/** Allocator to use in todds types. */
#if defined(TODDS_TBB_ALLOCATOR)
template<typename Type>
using allocator = tbb::scalable_allocator<Type>;
#elif defined(TODDS_MIMALLOC_ALLOCATOR)
template<typename Type>
using allocator = mi_stl_allocator<Type>;
#else
template<typename Type>
using allocator = std::allocator<Type>;
#endif

} // namespace todds
