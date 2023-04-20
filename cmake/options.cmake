# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

include_guard(GLOBAL)

if (TODDS_HYPERSCAN_SUPPORT)
	add_compile_definitions(TODDS_HYPERSCAN_SUPPORT)
endif ()

if (TODDS_TBB_ALLOCATOR)
	add_compile_definitions(TODDS_TBB_ALLOCATOR)
elseif (TODDS_MIMALLOC_ALLOCATOR)
	add_compile_definitions(TODDS_MIMALLOC_ALLOCATOR)
endif ()