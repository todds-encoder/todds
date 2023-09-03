/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/regex.hpp"

#include "todds/string.hpp"

#include <hs.h>

#include <cassert>

namespace {
hs_database* compile(std::string_view pattern, todds::string& error) {
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

hs_scratch* create_scratch(hs_database& database) {
	hs_scratch* scratch = nullptr;
	hs_alloc_scratch(&database, &scratch);
	return scratch;
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

class regex_pimpl final {
public:
	using scratch_ptr = std::unique_ptr<hs_scratch, void (*)(hs_scratch*)>;
	using database_ptr = std::unique_ptr<hs_database, void (*)(hs_database*)>;

	explicit regex_pimpl(std::string_view pattern)
		: _error{}
		, _database{compile(pattern, _error), free_database}
		, _scratch{nullptr, free_scratch} {
		_scratch = {_database != nullptr ? create_scratch(*_database) : nullptr, free_scratch};
	}

	[[nodiscard]] std::string_view error() const noexcept { return _error; }

	bool match(std::string_view input) {
		bool has_match{_database == nullptr || _scratch == nullptr};
		if (!has_match) {
			hs_scan(_database.get(), input.data(), static_cast<unsigned int>(input.size()), 0, _scratch.get(), handle_match,
				static_cast<void*>(&has_match));
		}
		return has_match;
	}

private:
	string _error;
	database_ptr _database;
	scratch_ptr _scratch;
};

regex::regex(std::string_view pattern)
	: _pimpl{std::make_unique<regex_pimpl>(pattern)} {}

regex::regex()
	: regex("") {}

regex::regex(regex&& other) noexcept = default;

regex& regex::operator=(regex&& other) noexcept = default;

regex::~regex() = default;

std::string_view regex::error() const noexcept { return _pimpl->error(); }

bool regex::match(std::string_view input) const { return _pimpl->match(input); }

} // namespace todds
