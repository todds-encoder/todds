/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "todds/string.hpp"

#include <oneapi/tbb/concurrent_queue.h>

namespace todds {

enum class report_type {
	/// todds has started retrieving all files to be encoded.
	retrieving_files_started,
	/// Retrieving files progress report.
	retrieving_files_progress,
	/// todds has finished retrieving all files. Contains the total time of this process.
	file_retrieval_time,
	/// File to process. Only enabled if the user specified verbose.
	file_verbose,
	/// The requested textures are being processed. This event is sent for both cleaning and encoding.
	process_started,
	/// Encoding progress report. Contains the number of textures encoded.
	encoding_progress,
	/// A non-critical error to be reported back to the user. Contains a text description of the error.
	pipeline_error,
};

class report final {
public:
	report() = default;
	explicit report(report_type type);
	report(report_type type, todds::string data);
	report(report_type type, std::size_t value);
	report(const report&) = delete;
	report(report&&) noexcept = default;
	report& operator=(const report&) = delete;
	report& operator=(report&&) noexcept = default;
	~report() = default;

	[[nodiscard]] report_type type() const;
	[[nodiscard]] const todds::string& data() const;
	[[nodiscard]] std::size_t value() const;

private:
	report_type _type{};
	todds::string _data{};
	std::size_t _value{};
};

using report_queue = oneapi::tbb::concurrent_queue<report>;

} // namespace todds
