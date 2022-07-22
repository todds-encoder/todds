/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/arguments.hpp"

#include "png2dds/project.hpp"

#include <boost/nowide/args.hpp>
#include <fmt/format.h>
#include <oneapi/tbb/info.h>

#include <algorithm>
#include <charconv>
#include <sstream>
#include <string_view>

namespace fs = boost::filesystem;

namespace {

// Maximum BC7 encoding quality level.
constexpr unsigned int max_level = 6U;
constexpr std::string_view level_arg = "--level";
constexpr std::string_view level_help =
	"Encoder quality level [0, 6]. Higher values provide better quality but take longer.";

constexpr std::string_view threads_arg = "--threads";
constexpr std::string_view threads_help_unformatted = "Number of threads used by the parallel pipeline [1, {:d}].";

constexpr std::string_view depth_arg = "--depth";
constexpr std::string_view depth_help = "Maximum subdirectory depth to use when looking for source files.";
constexpr std::size_t max_depth = std::numeric_limits<std::size_t>::max();

constexpr std::string_view overwrite_arg = "--overwrite";
constexpr std::string_view overwrite_help = "Convert files even if an output file already exists.";

constexpr std::string_view flip_arg = "--flip";
constexpr std::string_view flip_help = "Flip source images vertically before encoding.";

constexpr std::string_view time_arg = "--time";
constexpr std::string_view time_help = "Show the amount of time it takes to process all files.";

constexpr std::string_view regex_arg = "--regex";
constexpr std::string_view regex_help = "Process only absolute paths matching this regex.";

constexpr std::string_view help_arg = "--help";
constexpr std::string_view help_help = "Show usage information.";

constexpr std::string_view input_arg = "input";
constexpr std::string_view input_help =
	"Converts to DDS all PNG files inside of this folder. Can also point to a single PNG file.";

constexpr std::string_view output_arg = "output";
constexpr std::string_view output_help = "Create DDS files in this folder instead of next to their input PNG files.";

consteval std::size_t get_max_argument_size() {
	const std::vector<std::string_view> arg_names{
		level_arg, threads_arg, depth_arg, overwrite_arg, flip_arg, time_arg, regex_arg, help_arg, input_arg, output_arg};
	auto iterator = std::max_element(arg_names.cbegin(), arg_names.cend(),
		[](std::string_view lhs, std::string_view rhs) -> bool { return lhs.size() < rhs.size(); });

	return iterator->size();
}

template<typename Type>
void argument_from_str(
	std::string_view argument_name, std::string_view argument, Type& value, png2dds::args::data& parsed_arguments) {
	const auto [_, error] = std::from_chars(argument.data(), argument.data() + argument.size(), value);
	if (error == std::errc::invalid_argument || error == std::errc::result_out_of_range) {
		parsed_arguments.error = true;
		parsed_arguments.text = fmt::format("Argument error: {:s} must be a positive number.", argument_name);
	}
}

void print_argument(std::ostringstream& ostream, std::string_view arg_name, std::string_view arg_help) {
	constexpr std::size_t argument_table_size = get_max_argument_size() + 2UL;
	ostream << "  " << arg_name;
	std::fill_n(std::ostream_iterator<char>(ostream), argument_table_size - arg_name.size(), ' ');
	ostream << arg_help << '\n';
}

std::string get_help(std::size_t max_threads) {
	std::ostringstream ostream;
	ostream << png2dds::project::name() << ' ' << png2dds::project::version() << "\n\n"
					<< png2dds::project::description() << "\n\n";
	ostream << "USAGE:\n  " << png2dds::project::name() << " [OPTIONS] " << input_arg << " [" << output_arg << ']'
					<< "\n\n";

	ostream << "ARGS:\n";

	print_argument(ostream, input_arg, input_help);
	print_argument(ostream, output_arg, output_help);

	ostream << "\nOPTIONS:\n";
	print_argument(ostream, level_arg, level_help);
	const std::string threads_help = fmt::format(threads_help_unformatted, max_threads);
	print_argument(ostream, threads_arg, threads_help);
	print_argument(ostream, depth_arg, depth_help);
	print_argument(ostream, overwrite_arg, overwrite_help);
	print_argument(ostream, flip_arg, flip_help);
	print_argument(ostream, time_arg, time_help);
	print_argument(ostream, regex_arg, regex_help);
	print_argument(ostream, help_arg, help_help);

	return std::move(ostream).str();
}

} // anonymous namespace

namespace png2dds::args {

data get(int argc, char** argv) {
	boost::nowide::args nowide_args(argc, argv);
	std::vector<std::string_view> arguments;
	arguments.reserve(static_cast<std::size_t>(argc));
	for (std::size_t index = 0UL; index < static_cast<std::size_t>(argc); ++index) {
		arguments.emplace_back(argv[index]);
	}

	return get(arguments);
}

data get(const std::vector<std::string_view>& arguments) {
	constexpr std::string_view optional_prefix{"--"};

	data parsed_arguments{};
	// Set default values.
	parsed_arguments.level = max_level;
	const auto max_threads = static_cast<std::size_t>(oneapi::tbb::info::default_concurrency());
	parsed_arguments.threads = max_threads;
	parsed_arguments.depth = max_depth;

	std::size_t index = 1UL;

	// Parse all positional arguments.
	while (!parsed_arguments.error && index < arguments.size()) {
		const std::string_view argument = arguments[index];
		if (!argument.starts_with(optional_prefix)) { break; }
		if (argument == optional_prefix) {
			++index;
			break;
		}

		if (argument == help_arg) {
			parsed_arguments.text = get_help(max_threads);
			break;
		}

		const std::string_view next_argument = (index < arguments.size() - 1UL) ? arguments[index + 1UL] : "";
		if (argument == level_arg) {
			++index;
			argument_from_str(level_arg, next_argument, parsed_arguments.level, parsed_arguments);
			if (!parsed_arguments.error && parsed_arguments.level > max_level) {
				parsed_arguments.error = true;
				parsed_arguments.text =
					fmt::format("Argument error: Unsupported encode quality level {:d}.", parsed_arguments.level);
			}
		} else if (argument == threads_arg) {
			++index;
			argument_from_str(threads_arg, next_argument, parsed_arguments.threads, parsed_arguments);
			parsed_arguments.threads = std::clamp<std::size_t>(parsed_arguments.threads, 1ULL, max_threads);
		} else if (argument == depth_arg) {
			++index;
			argument_from_str(depth_arg, next_argument, parsed_arguments.depth, parsed_arguments);
			if (parsed_arguments.depth == 0UL) { parsed_arguments.depth = max_depth; }
		} else if (argument == regex_arg) {
			++index;
			parsed_arguments.regex = png2dds::regex{next_argument};
			const auto regex_err = parsed_arguments.regex.error();
			if (!regex_err.empty()) {
				parsed_arguments.error = true;
				parsed_arguments.text =
					fmt::format("Could not compile regular expression {:s}: {:s}", next_argument, regex_err);
			}
		} else if (argument == overwrite_arg) {
			parsed_arguments.overwrite = true;
		} else if (argument == flip_arg) {
			parsed_arguments.flip = true;
		} else if (argument == time_arg) {
			parsed_arguments.time = true;
		} else {
			parsed_arguments.error = true;
			parsed_arguments.text = fmt::format("Invalid positional argument {:s}", argument);
		}

		++index;
	}

	if (parsed_arguments.text.empty()) {
		if (index < arguments.size()) {
			boost::system::error_code error_code;
			parsed_arguments.input = fs::canonical(arguments[index].data(), error_code);
			if (error_code) {
				parsed_arguments.error = true;
				parsed_arguments.text =
					fmt::format("Invalid {:s} {:s}: {:s}.", input_arg, arguments[index], error_code.message());
			} else if (index < arguments.size() - 1UL) {
				parsed_arguments.output = arguments[index + 1UL].data();
			}
		} else if (index == 1UL) {
			// No arguments provided.
			parsed_arguments.text = get_help(max_threads);
		} else {
			parsed_arguments.error = true;
			parsed_arguments.text = fmt::format("Argument error: {:s} has not been provided.", input_arg);
		}
	}

	return parsed_arguments;
}

} // namespace png2dds::args
