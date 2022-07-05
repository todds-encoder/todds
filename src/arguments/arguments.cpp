/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/arguments.hpp"

#include "png2dds/project.hpp"

#include <boost/nowide/args.hpp>
#include <cxxopts.hpp>
#include <fmt/format.h>

#include <string_view>
#include <thread>

namespace png2dds::args {

data get(int argc, char** argv) {
	boost::nowide::args nowide_args(argc, argv);

	data arguments;

	try {
		cxxopts::Options options(std::string{project::name()}, std::string{project::description()});

		const std::string& path_arg = "path";
		const std::string& path_help = "Convert to DDS all PNG files inside of this folder";

		const std::string& list_arg = "list";
		const std::string& list_help =
			"Points to a text file with a list of PNG files and/or directories. Entries must be on separate lines. All "
			"individual PNG files and those inside specified directories will be converted to DDS.";

		const std::string& threads_arg = "threads";
		const std::string& threads_help = "Number of threads that png2dds will use";

		const std::string& depth_arg = "depth";
		const std::string& depth_help = "Maximum subdirectory depth to look for PNG files";

		const std::string& overwrite_arg = "overwrite";
		const std::string& overwrite_help = "Convert PNG files to DDS even if they already have a DDS file";

		const std::string& help_arg = "help";
		const std::string& help_help = "Show usage information";

		const std::string& version_arg = "version";
		const std::string& version_help = "Show program version";

		const unsigned int max_threads = std::thread::hardware_concurrency();
		// clang-format off
		options.add_options()
			(path_arg, path_help, cxxopts::value<std::string>()->default_value(""))
			(list_arg, list_help, cxxopts::value<std::string>()->default_value(""))
			(threads_arg, threads_help, cxxopts::value<unsigned int>()->default_value(std::to_string(max_threads)))
			(depth_arg, depth_help, cxxopts::value<unsigned int>())
			(overwrite_arg, overwrite_help)
			(help_arg, help_help)
			(version_arg, version_help);
		// clang-format on

		auto result = options.parse(argc, argv);

		if (result.count(help_arg) > 0U) {
			arguments.text = options.help();
		} else if (result.count(version_arg) > 0U) {
			arguments.text = fmt::format("{:s} {:s}", project::name(), project::version());
		} else {
			arguments.path = result[path_arg].as<std::string>();
			arguments.list = result[list_arg].as<std::string>();
			arguments.threads = std::min(result[threads_arg].as<unsigned int>(), max_threads);
			arguments.depth =
				result.count(depth_arg) > 0 ? result[depth_arg].as<unsigned int>() : std::numeric_limits<unsigned int>::max();
			arguments.overwrite = result.count(overwrite_arg) > 0;

			if (arguments.path.empty() && arguments.list.empty()) {
				arguments.error = true;
				arguments.text = fmt::format("Must provide {:s} and/or {:s}.", path_arg, list_arg);
			} else if (arguments.threads == 0U) {
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
