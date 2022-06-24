/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/arguments.hpp"

#include "png2dds/project.hpp"

#include <argparse/argparse.hpp>
#include <boost/nowide/args.hpp>
#include <fmt/format.h>

#include <string_view>
#include <thread>

namespace png2dds::args {

constexpr std::string_view path_arg = "path";
constexpr std::string_view only_arg = "--only";
constexpr std::string_view threads_arg = "--threads";

data get(int argc, char** argv) {
	boost::nowide::args _(argc, argv);

	argparse::ArgumentParser program(std::string{project::name()}, std::string{project::version()});

	program.add_description(std::string{project::description()});

	program.add_argument(path_arg).help(
		"Convert PNG files in this path. DDS files will be created next to their PNG counterpart.");

	program.add_argument(only_arg).help("Only convert PNGs that contain a folder with the specified folder in its path. "
																			"This comparison is case insensitive.");

	program.add_argument(threads_arg)
		.default_value(static_cast<int>(std::thread::hardware_concurrency()))
		.scan<'i', int>()
		.help("Number of threads to use. If not used png2dds will use the maximum number of threads.");

	data result;
	try {
		program.parse_args(argc, argv);
		result.path = program.get(path_arg);
		result.only = program.present(only_arg).value_or("");
		result.threads = program.get<int>(threads_arg);
	} catch (const std::runtime_error& ex) {
		result.error = fmt::format("{:s}\n{:s}", ex.what(), program.help().str());
	} catch (const std::logic_error& ex) {
		result.error = fmt::format("Argument error: {:s}\n{:s}", ex.what(), program.help().str());
	}

	return result;
}

} // namespace png2dds::args
