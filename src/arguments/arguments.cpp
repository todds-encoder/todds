/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/arguments.hpp"

#include "png2dds/project.hpp"

#include <boost/nowide/args.hpp>
#include <boost/program_options.hpp>
#include <fmt/format.h>
#include <oneapi/tbb/info.h>

#include <iomanip>
#include <ostream>
#include <string_view>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace {
// Maximum BC7 encoding quality level.
constexpr unsigned int max_level = 6U;
constexpr std::string_view level_arg = "level";
constexpr std::string_view level_help =
	"Encoder quality level [0, 6]. Higher values provide better quality but take longer.";

constexpr std::string_view threads_arg = "threads";
constexpr std::string_view threads_help_unformatted =
	"Total number of threads used by the parallel pipeline [1, {:d}].";

constexpr std::string_view depth_arg = "depth";
constexpr std::string_view depth_help = "Maximum subdirectory depth to use when looking for source files.";

constexpr std::string_view overwrite_arg = "overwrite";
constexpr std::string_view overwrite_help = "Convert files even if an output file already exists.";

constexpr std::string_view flip_arg = "flip";
constexpr std::string_view flip_help = "Flip source images vertically before encoding.";

constexpr std::string_view time_arg = "time";
constexpr std::string_view time_help = "Show the amount of time it takes to process all files.";

constexpr std::string_view help_arg = "help";
constexpr std::string_view help_help = "Show usage information.";

constexpr std::string_view input_arg = "input";
constexpr std::string_view input_help =
	"Converts to DDS all PNG files inside of this folder. Can also point to a single PNG file.";

constexpr std::string_view output_arg = "output";
constexpr std::string_view output_help = "By default DDS files are created next to their input PNG files. If this "
																				 "argument is provided, DDS files will be created in this folder instead.";

std::string get_help(const po::options_description& description) {
	std::ostringstream ostream;
	ostream << png2dds::project::name() << ' ' << png2dds::project::version() << "\n\n"
					<< png2dds::project::description() << "\n\n";
	ostream << "USAGE:\n  " << png2dds::project::name() << " [OPTIONS] " << input_arg << " [" << output_arg << ']'
					<< "\n\n";

	ostream << "ARGS:\n";

	const unsigned int option_size = description.get_option_column_width();
	ostream << "  " << input_arg;
	std::fill_n(std::ostream_iterator<char>(ostream), option_size - 2UL - input_arg.size(), ' ');
	ostream << input_help << '\n';

	ostream << "  " << output_arg;
	std::fill_n(std::ostream_iterator<char>(ostream), option_size - 2UL - output_arg.size(), ' ');
	ostream << output_help << "\n\n";

	description.print(ostream);
	return std::move(ostream).str();
}

} // anonymous namespace

namespace png2dds::args {

data get(int argc, char** argv) {
	boost::nowide::args nowide_args(argc, argv);

	// Optional arguments are defined separately to print only then when --help is used.
	po::options_description optional_arguments("OPTIONS", 200, 50);
	const auto max_threads = static_cast<std::size_t>(oneapi::tbb::info::default_concurrency());
	const std::string threads_help = fmt::format(threads_help_unformatted, max_threads);

	data arguments{};
	bool help{};
	std::string input_str;
	std::string output_str;
	optional_arguments.add_options()
		// clang-format off
		(level_arg.data(), po::value<unsigned int>(&arguments.level)->default_value(max_level), level_help.data())
		(threads_arg.data(), po::value<std::size_t>(&arguments.threads)->default_value(max_threads), threads_help.c_str())
		(depth_arg.data(), po::value<std::size_t>(&arguments.depth), depth_help.data())
		(overwrite_arg.data(), po::bool_switch(&arguments.overwrite),overwrite_help.data())
		(flip_arg.data(), po::bool_switch(&arguments.flip),flip_help.data())
		(time_arg.data(), po::bool_switch(&arguments.time),time_help.data())
		(help_arg.data(), po::bool_switch(&help), help_help.data());

	// clang-format on

	po::options_description all_arguments;
	all_arguments.add(optional_arguments);
	all_arguments.add_options()
		// clang-format off
		(input_arg.data(), po::value<std::string>(&input_str))
		(output_arg.data(), po::value<std::string>(&output_str));
	// clang-format on

	po::positional_options_description positional_options;
	positional_options.add(input_arg.data(), 1);
	positional_options.add(output_arg.data(), 1);
	try {
		const auto parsed_options =
			po::command_line_parser(argc, argv).options(all_arguments).positional(positional_options).run();
		po::variables_map variables;
		store(parsed_options, variables, true);
		notify(variables);
	} catch (const boost::program_options::error& exc) {
		arguments.error = true;
		arguments.text = fmt::format("Argument error: {:s}.", exc.what());
		return arguments;
	}

	if (help) {
		arguments.text = get_help(optional_arguments);
	} else if (arguments.level > max_level) {
		arguments.error = true;
		arguments.text = fmt::format("Argument error: Unsupported encode quality level {:d}.", arguments.level);
	} else if (input_str.empty()) {
		arguments.error = true;
		arguments.text = fmt::format("Argument error: {:s} has not been provided.", input_arg);
	}

	arguments.threads = std::clamp(arguments.threads, 1UL, max_threads);
	if (arguments.depth == 0UL) { arguments.depth = std::numeric_limits<unsigned int>::max(); }

	boost::system::error_code error_code;
	arguments.input = fs::canonical(input_str, error_code);
	if (error_code) {
		arguments.error = true;
		arguments.text = fmt::format("Invalid input {:s}: {:s}.", input_str, error_code.message());
	} else if (!output_str.empty()) {
		arguments.output = output_str;
	}

	return arguments;
}

} // namespace png2dds::args
