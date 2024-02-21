/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <string_view>

namespace todds {

/** RAII opaque wrapper around a Hyperscan database.
 * This type is NOT thread-safe.
 */
class regex final {
public:
	regex();
	/**
	 * Compiles a regular expression database which will only generate a single match per stream.
	 * @param pattern Regular expression to be parsed encoded as UTF-8.
	 */
	explicit regex(std::string_view pattern);

	regex(const regex&) = delete;
	regex(regex&& other) noexcept;
	regex& operator=(const regex&) = delete;
	regex& operator=(regex&& other) noexcept;
	~regex();

	/**
	 * Contains compilations errors. If there weren't any, it is empty.
	 * @return Error string.
	 */
	[[nodiscard]] std::string_view error() const noexcept;

	/**
	 * Checks if the regex instance is properly set-up.
	 * @return True if the regex has been initialized with a pattern.
	 */
	[[nodiscard]] bool valid() const noexcept;

	/**
	 * Checks if an input matches the regular expression compiled into a database.
	 * This operation is not thread safe. Should never be called on invalid regex instances.
	 * @param input Input to be checked for matches, encoded in UTF-8.
	 * @return True if a match was found or if the internal database is not valid.
	 */
	[[nodiscard]] bool match(std::string_view input) const;

private:
	std::unique_ptr<class regex_pimpl> _pimpl;
};

} // namespace todds
