/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <string_view>

// Forward declaration of required Hyperscan types.
struct hs_database;
struct hs_scratch;

namespace todds {

/** RAII wrapper around a Hyperscan database. */
class regex final {
public:
	using scratch_type = std::unique_ptr<hs_scratch, void (*)(hs_scratch*)>;
	regex();
	/**
	 * Compiles a regular expression database which will only generate a single match per stream.
	 * @param pattern Regular expression to be parsed encoded as UTF-8.
	 */
	explicit regex(std::string_view pattern);

	regex(const regex&) = delete;
	regex(regex&&) noexcept = default;
	regex& operator=(const regex&) = delete;
	regex& operator=(regex&&) noexcept = default;
	~regex() = default;

	/**
	 * Pointer to the internal database used by Hyperscan. May be null if the regex was built without a pattern or if
	 * any compile errors happened.
	 * @return Database.
	 */
	[[nodiscard]] const hs_database* database() const noexcept;

	/**
	 * Scratch required to evaluate this regex. May be null if the regex was built without a pattern or if
	 * any compile errors happened.
	 * @return Newly allocated scratch.
	 */
	[[nodiscard]] scratch_type allocate_scratch() const;

	/**
	 * Contains compilations errors. If there weren't any, it is empty.
	 * @return Error string.
	 */
	[[nodiscard]] std::string_view error() const noexcept;

	/**
	 * Checks if an input matches the regular expression compiled into a database.
	 * @param scratch Scratch to use during evaluation. Must have been created by this regex.
	 * @param input Input to be checked for matches.
	 * @return True if a match was found, if the internal database is null, or if the scratch is null.
	 */
	[[nodiscard]] bool match(scratch_type& scratch, std::string_view input) const;

private:
	std::string _error;
	std::unique_ptr<hs_database, void (*)(hs_database*)> _database;
};

} // namespace todds
