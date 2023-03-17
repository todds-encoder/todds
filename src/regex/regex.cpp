/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/regex.hpp"

#include <hs.h>

#include <cassert>

namespace {
hs_database* compile(std::string_view pattern, std::string& error) {
	hs_database* database{};

	if (!pattern.empty()) {
		constexpr unsigned int compile_flags = HS_FLAG_SINGLEMATCH | HS_FLAG_UTF8;
		assert(error.empty());
		hs_compile_error_t* hs_error{};
		if (hs_compile(pattern.data(), compile_flags, HS_MODE_BLOCK, nullptr, &database, &hs_error) != HS_SUCCESS) {
			error = hs_error->message;
			hs_free_compile_error(hs_error);
		}
	}

	return database;
}

void free_database(hs_database* database) {
	assert(database != nullptr);
	hs_free_database(database);
}

void free_scratch(hs_scratch* scratch) {
	assert(scratch != nullptr);
	hs_free_scratch(scratch);
}

int handle_match(
	unsigned int /*id*/, unsigned long long /*from*/, unsigned long long /*to*/, unsigned int /*flags*/, void* context) {
	bool& has_match = *static_cast<bool*>(context);
	has_match = true;
	return 0;
}

} // namespace

namespace todds {

regex::regex()
	: _error{}
	, _database{nullptr, free_database} {}

regex::regex(std::string_view pattern)
	: _error{}
	, _database{compile(pattern, _error), free_database} {}

const hs_database* regex::database() const noexcept { return _database.get(); }

std::string_view regex::error() const noexcept { return _error; }

regex::scratch_type regex::allocate_scratch() const {
	hs_scratch* scratch = nullptr;
	if (_database) { hs_alloc_scratch(_database.get(), &scratch); }
	return {scratch, free_scratch};
}

bool regex::match(regex::scratch_type& scratch, std::string_view input) const {
	bool has_match{_database == nullptr || scratch == nullptr};
	if (!has_match) {
		hs_scan(_database.get(), input.data(), static_cast<unsigned int>(input.size()), 0, scratch.get(), handle_match,
			static_cast<void*>(&has_match));
	}
	return has_match;
}

} // namespace todds
