/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/arguments.hpp"

#include "png2dds/format.hpp"
#include "png2dds/project.hpp"
#include "png2dds/vector.hpp"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/nowide/args.hpp>
#include <fmt/format.h>
#include <oneapi/tbb/info.h>

#include <algorithm>
#include <charconv>
#include <sstream>
#include <string_view>

namespace fs = boost::filesystem;

namespace {

struct optional_arg {
	std::string_view name{};
	std::string_view shorter{};
	std::string_view help{};
};

consteval optional_arg optional_argument(std::string_view arg_name, std::string_view help) {
	return optional_arg{arg_name, std::string_view{arg_name.data() + 1U, arg_name.data() + 3}, help};
}

constexpr bool matches(std::string_view argument, const optional_arg& optional_arg) {
	return argument == optional_arg.shorter || argument == optional_arg.name;
}

constexpr auto format_arg =
	optional_argument("--format", "DDS encoding format. BC7 is used if this parameter is not used.");

constexpr auto level_arg = optional_argument("--level", "Encoder quality level. Higher values take more time.");

constexpr auto threads_arg =
	optional_arg{"--threads", "-th", "Number of threads used by the parallel pipeline [1, {:d}]."};

constexpr std::size_t max_depth = std::numeric_limits<std::size_t>::max();
constexpr auto depth_arg =
	optional_argument("--depth", "Maximum subdirectory depth to use when looking for source files.");

constexpr auto overwrite_arg = optional_argument("--overwrite", "Convert files even if an output file already exists.");

constexpr auto vflip_arg = optional_arg{"--vflip", "-vf", "Flip source images vertically before encoding."};

constexpr auto time_arg = optional_argument("--time", "Show total execution time.");

constexpr auto regex_arg = optional_argument("--regex", "Process only absolute paths matching this regex.");

constexpr auto verbose_arg = optional_argument("--verbose", "Display progress messages.");

constexpr auto help_arg = optional_argument("--help", "Show usage information.");

constexpr std::string_view input_name = "input";
constexpr std::string_view input_help =
	"Converts to DDS all PNG files inside of this folder. Can also point to a single PNG file.";

constexpr std::string_view output_name = "output";
constexpr std::string_view output_help = "Create DDS files in this folder instead of next to their input PNG files.";

consteval std::size_t get_max_argument_size() {
	// Other implementations may not have constexpr support so std::vector is used explicitly.
	const std::vector<std::string_view> arg_names{level_arg.name, threads_arg.name, depth_arg.name, overwrite_arg.name,
		vflip_arg.name, time_arg.name, verbose_arg.name, regex_arg.name, help_arg.name, input_name, output_name};
	auto iterator = std::max_element(arg_names.cbegin(), arg_names.cend(),
		[](std::string_view lhs, std::string_view rhs) -> bool { return lhs.size() < rhs.size(); });

	return iterator->size();
}
void print_argument_impl(
	std::ostringstream& ostream, std::string_view arg_shorter, std::string_view arg_name, std::string_view arg_help) {
	constexpr std::size_t argument_table_size = get_max_argument_size() + 2UL;
	ostream << "  ";
	if (!arg_shorter.empty()) { ostream << arg_shorter << ", "; }
	ostream << arg_name;
	std::fill_n(std::ostream_iterator<char>(ostream), argument_table_size - arg_name.size(), ' ');
	ostream << arg_help << '\n';
}

void print_positional_argument(std::ostringstream& ostream, std::string_view arg_name, std::string_view arg_help) {
	print_argument_impl(ostream, "", arg_name, arg_help);
}

void print_optional_argument(std::ostringstream& ostream, const optional_arg& argument) {
	print_argument_impl(ostream, argument.shorter, argument.name, argument.help);
}

void print_format_information(std::ostringstream& ostream, png2dds::format::type format_type) {
	constexpr std::size_t argument_space = get_max_argument_size() + 4UL;
	std::fill_n(std::ostream_iterator<char>(ostream), argument_space, ' ');
	ostream << fmt::format("{:s}: Encoder quality level [0, {:d}]\n", png2dds::format::name(format_type),
		png2dds::format::max_level(format_type));
}

std::string get_help(std::size_t max_threads) {
	std::ostringstream ostream;
	ostream << png2dds::project::name() << ' ' << png2dds::project::version() << "\n\n"
					<< png2dds::project::description() << "\n\n";
	ostream << "USAGE:\n  " << png2dds::project::name() << " [OPTIONS] " << input_name << " [" << output_name << ']'
					<< "\n\n";

	ostream << "ARGS:\n";

	print_positional_argument(ostream, input_name, input_help);
	print_positional_argument(ostream, output_name, output_help);

	ostream << "\nOPTIONS:\n";
	print_optional_argument(ostream, format_arg);
	print_format_information(ostream, png2dds::format::type::bc1);
	print_format_information(ostream, png2dds::format::type::bc7);

	print_optional_argument(ostream, level_arg);
	const std::string threads_help = fmt::format(threads_arg.help, max_threads);
	print_argument_impl(ostream, threads_arg.shorter, threads_arg.name, threads_help);
	print_optional_argument(ostream, depth_arg);
	print_optional_argument(ostream, overwrite_arg);
	print_optional_argument(ostream, vflip_arg);
	print_optional_argument(ostream, time_arg);
	print_optional_argument(ostream, regex_arg);
	print_optional_argument(ostream, verbose_arg);
	print_optional_argument(ostream, help_arg);

	return std::move(ostream).str();
}

void format_from_str(std::string_view argument, png2dds::args::data& parsed_arguments) {
	const std::string argument_upper = boost::to_upper_copy(std::string{argument});
	if (argument_upper == png2dds::format::name(png2dds::format::type::bc1)) {
		parsed_arguments.format = png2dds::format::type::bc1;
	} else if (argument_upper == png2dds::format::name(png2dds::format::type::bc7)) {
		parsed_arguments.format = png2dds::format::type::bc7;
	} else {
		parsed_arguments.error = true;
		parsed_arguments.text = fmt::format("Unsupported format: {:s}", argument);
	}
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

} // anonymous namespace

namespace png2dds::args {

data get(int argc, char** argv) {
	boost::nowide::args nowide_args(argc, argv);
	png2dds::vector<std::string_view> arguments;
	arguments.reserve(static_cast<std::size_t>(argc));
	for (std::size_t index = 0UL; index < static_cast<std::size_t>(argc); ++index) {
		arguments.emplace_back(argv[index]);
	}

	return get(arguments);
}

data get(const png2dds::vector<std::string_view>& arguments) {
	data parsed_arguments{};
	// Set default values. The default value of level is set after parsing format.
	parsed_arguments.format = format::type::bc7;
	const auto max_threads = static_cast<std::size_t>(oneapi::tbb::info::default_concurrency());
	parsed_arguments.threads = max_threads;
	parsed_arguments.depth = max_depth;

	std::size_t index = 1UL;

	bool level_is_set = false;

	// Parse all positional arguments.
	while (!parsed_arguments.error && index < arguments.size()) {
		const std::string_view argument = arguments[index];
		constexpr std::string_view optional_prefix{"-"};
		if (!argument.starts_with(optional_prefix)) { break; }
		constexpr std::string_view positional_start{"--"};
		if (argument == positional_start) {
			++index;
			break;
		}

		if (matches(argument, help_arg)) {
			parsed_arguments.text = get_help(max_threads);
			break;
		}

		const std::string_view next_argument = (index < arguments.size() - 1UL) ? arguments[index + 1UL] : "";
		if (matches(argument, format_arg)) {
			++index;
			format_from_str(next_argument, parsed_arguments);
		} else if (matches(argument, level_arg)) {
			level_is_set = true;
			++index;
			argument_from_str(level_arg.name, next_argument, parsed_arguments.level, parsed_arguments);
		} else if (matches(argument, threads_arg)) {
			++index;
			argument_from_str(threads_arg.name, next_argument, parsed_arguments.threads, parsed_arguments);
			parsed_arguments.threads = std::clamp<std::size_t>(parsed_arguments.threads, 1ULL, max_threads);
		} else if (matches(argument, depth_arg)) {
			++index;
			argument_from_str(depth_arg.name, next_argument, parsed_arguments.depth, parsed_arguments);
			if (parsed_arguments.depth == 0UL) { parsed_arguments.depth = max_depth; }
		} else if (matches(argument, regex_arg)) {
			++index;
			parsed_arguments.regex = png2dds::regex{next_argument};
			const auto regex_err = parsed_arguments.regex.error();
			if (!regex_err.empty()) {
				parsed_arguments.error = true;
				parsed_arguments.text =
					fmt::format("Could not compile regular expression {:s}: {:s}", next_argument, regex_err);
			}
		} else if (matches(argument, overwrite_arg)) {
			parsed_arguments.overwrite = true;
		} else if (matches(argument, vflip_arg)) {
			parsed_arguments.vflip = true;
		} else if (matches(argument, time_arg)) {
			parsed_arguments.time = true;
		} else if (matches(argument, verbose_arg)) {
			parsed_arguments.verbose = true;
		} else {
			parsed_arguments.error = true;
			parsed_arguments.text = fmt::format("Invalid positional argument {:s}", argument);
		}

		++index;
	}

	if (!parsed_arguments.error) {
		const auto format_max_level = max_level(parsed_arguments.format);
		if (!level_is_set) {
			parsed_arguments.level = format_max_level;
		} else if (parsed_arguments.level > format_max_level) {
			parsed_arguments.error = true;
			parsed_arguments.text =
				fmt::format("Argument error: Unsupported encode quality level {:d} .", parsed_arguments.level);
		}
	}

	if (parsed_arguments.text.empty()) {
		if (index < arguments.size()) {
			boost::system::error_code error_code;
			parsed_arguments.input = fs::canonical(arguments[index].data(), error_code);
			if (error_code) {
				parsed_arguments.error = true;
				parsed_arguments.text =
					fmt::format("Invalid {:s} {:s}: {:s}.", input_name, arguments[index], error_code.message());
			} else if (index < arguments.size() - 1UL) {
				parsed_arguments.output = arguments[index + 1UL].data();
			}
		} else if (index == 1UL) {
			// No arguments provided.
			parsed_arguments.text = get_help(max_threads);
		} else {
			parsed_arguments.error = true;
			parsed_arguments.text = fmt::format("Argument error: {:s} has not been provided.", input_name);
		}
	}

	return parsed_arguments;
}

} // namespace png2dds::args
