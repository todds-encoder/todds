/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/arguments.hpp"

#include "png2dds/project.hpp"

#include <boost/nowide/args.hpp>
#include <cxxopts.hpp>
#include <fmt/format.h>

#include <algorithm>
#include <string_view>
#include <thread>

namespace png2dds::args {

data get(int argc, char** argv) {
	boost::nowide::args _(argc, argv);

	data arguments;

	try {
		cxxopts::Options options(std::string{project::name()}, std::string{project::description()});

		const std::string& help_arg = "help";
		const std::string& help_help = "Show usage information";

		const std::string& threads_arg = "threads";
		const std::string& threads_help = "Number of threads that png2dds will use";

		const std::string& path_arg = "path";
		const std::string& path_help = "Convert all PNG files inside of this folder";

		const unsigned int max_threads = std::thread::hardware_concurrency();
		options.add_options()(
			threads_arg, threads_help, cxxopts::value<unsigned int>()->default_value(std::to_string(max_threads)))(
			path_arg, path_help, cxxopts::value<std::string>())(help_arg, help_help);

		auto result = options.parse(argc, argv);

		if (result.count(help_arg) > 0U) {
			arguments.text = options.help();
		} else {
			arguments.threads = std::min(result[threads_arg].as<unsigned int>(), max_threads);

			if (arguments.threads == 0U) {
				arguments.error = true;
				arguments.text = fmt::format("{:s} must be greater than zero.", threads_arg);
			}
		}

	} catch (const cxxopts::OptionException& ex) {
		arguments.error = true;
		arguments.text = ex.what();
	}

	return arguments;
}

} // namespace png2dds::args
