/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "include/todds/report.hpp"

namespace todds {

report::report(report_type type)
	: _type{type}
	, _data{}
	, _value{} {}

report::report(report_type type, todds::string data)
	: _type{type}
	, _data{std::move(data)}
	, _value{} {}

report::report(report_type type, std::size_t value)
	: _type{type}
	, _data{}
	, _value{value} {}

report_type report::type() const { return _type; }

const todds::string& report::data() const { return _data; }

std::size_t report::value() const { return _value; }

} // namespace todds
