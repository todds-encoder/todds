/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#if defined(TRACY_ENABLE)
#include <tracy/Tracy.hpp>
#endif

#include <memory>

#if defined(TODDS_TBB_ALLOCATOR)
#include <oneapi/tbb/scalable_allocator.h>
#elif defined(TODDS_MIMALLOC_ALLOCATOR)
#include <mimalloc.h>
#endif

namespace todds {

/** Allocator to use in todds types. */
#if defined(TODDS_TBB_ALLOCATOR)
template<typename Type> using allocator = tbb::scalable_allocator<Type>;
#elif defined(TODDS_MIMALLOC_ALLOCATOR)
template<typename Type> using allocator = mi_stl_allocator<Type>;
#elif defined(TRACY_ENABLE)

/** @brief Conformant allocator with Tracy Profiler support.
 *
 * This allocator does not handle any exceptions and terminates on allocation failure.
 */
template<typename T> class allocator {
public:
	/** @brief Value type being allocated by this allocator. */
	using value_type = T;

	/** @brief Allow rebinding the allocator to other types. */
	template<class U> constexpr explicit allocator(const allocator<U>& /*other*/) noexcept {}

	/** @brief Default constructor. */
	[[nodiscard]] constexpr allocator() noexcept = default;

	/** @brief Copy constructor. */
	[[nodiscard]] constexpr allocator(const allocator&) noexcept = default;

	/** @brief Move constructor. */
	[[nodiscard]] constexpr allocator(allocator&&) noexcept = default;

	/** @brief Copy assignment operator. */
	constexpr allocator& operator=(const allocator&) noexcept = default;

	/** @brief Move assignment operator. */
	constexpr allocator& operator=(allocator&&) noexcept = default;

	/** @brief Default destructor. */
	~allocator() = default;

	/** @brief Allocates memory for n instances of T.
	 *
	 * Terminates on allocation failure.
	 */
	[[nodiscard]] T* allocate(std::size_t n) noexcept {
		void* memory = std::malloc(n * sizeof(T)); // NOLINT
		if (memory == nullptr) [[unlikely]] { std::abort(); }
		TracyAlloc(memory, n * sizeof(T));
		return static_cast<T*>(memory);
	}

	/** @brief Deallocates the memory of n instances of T. */
	void deallocate(T* memory, std::size_t /* n */) noexcept {
		TracyFree(memory);
		std::free(memory); // NOLINT
	}
};

/** @brief Equality comparison operator. */
template<typename T1, typename T2>
[[nodiscard]] bool operator==(const allocator<T1>& /* lh */, const allocator<T2>& /* rh */) noexcept {
	return true;
}

/** @brief Inequality comparison operator. */
template<typename T1, typename T2>
[[nodiscard]] bool operator!=(const allocator<T1>& /* lh */, const allocator<T2>& /* rh */) noexcept {
	return false;
}

#else
template<typename Type> using allocator = std::allocator<Type>;
#endif

} // namespace todds
