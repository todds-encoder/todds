# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

include_guard(GLOBAL)

if (TODDS_ISPC)
	add_compile_definitions(TODDS_ISPC)
endif ()

if (TODDS_PIPELINE_DUMP)
	add_compile_definitions(TODDS_PIPELINE_DUMP)
endif ()

if (TODDS_REGULAR_EXPRESSIONS)
	add_compile_definitions(TODDS_REGULAR_EXPRESSIONS)
endif ()

if (TODDS_TBB_ALLOCATOR)
	add_compile_definitions(TODDS_TBB_ALLOCATOR)
elseif (TODDS_MIMALLOC_ALLOCATOR)
	add_compile_definitions(TODDS_MIMALLOC_ALLOCATOR)
endif ()