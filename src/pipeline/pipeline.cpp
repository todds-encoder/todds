/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/pipeline.hpp"

#include "todds/dds.hpp"
#include "todds/string.hpp"

#include <boost/nowide/iostream.hpp>
#include <fmt/format.h>
#include <oneapi/tbb/global_control.h>
#include <oneapi/tbb/parallel_pipeline.h>

#include <atomic>

#include "filter_common.hpp"
#include "get_filters_from_settings.hpp"

namespace otbb = oneapi::tbb;
using todds::dds_image;
using todds::pipeline::paths_vector;
namespace todds::pipeline {

void encode_as_dds(const input& input_data, std::atomic<bool>& force_finish, report_queue& updates) {
	dds::initialize_encoding(input_data.format, input_data.alpha_format);

	// Ensure that OpenCV is working in sequential mode.
	cv::setNumThreads(0);
	// Setup the parallel pipeline.
	const otbb::global_control control(otbb::global_control::max_allowed_parallelism, input_data.parallelism);
	// Maximum number of files that the pipeline can process at the same time.
	const std::size_t tokens = input_data.parallelism * 4UL;

	// Used to give each file processed in a token a unique file index to access the paths and files_data vectors.
	std::atomic<std::size_t> counter;
	// Contains extra data about each file being processed.
	// Pipeline stages may write or read from this vector at any time. Since each token has a unique index, these
	// accesses are thread-safe.
	vector<impl::file_data> files_data(input_data.paths.size());

	const otbb::filter<void, void> filters =
		get_filters_from_settings(input_data, counter, force_finish, updates, files_data);

	otbb::parallel_pipeline(tokens, filters);

	if (input_data.report) {
		// Reports are not supported by the report system at the moment.
		boost::nowide::cout << "File;Width;Height;Mipmaps;Format\n";
		for (std::size_t index = 0U; index < input_data.paths.size(); ++index) {
			const string& dds_path = input_data.paths[index].second.string();
			const auto& data = files_data[index];
			boost::nowide::cout << fmt::format(
				"{:s};{:d};{:d};{:d};{:s}\n", dds_path, data.width, data.height, data.mipmaps, format::name(data.format));
		}
	}
}

} // namespace todds::pipeline
