/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "png2dds/arguments.hpp"
#include "png2dds/format.hpp"
#include "png2dds/project.hpp"

#include <boost/filesystem/operations.hpp>
#include <oneapi/tbb/info.h>

#include <limits>

#include <catch2/catch_test_macros.hpp>

using png2dds::args::get;

constexpr auto binary = png2dds::project::name();

constexpr std::string_view utf_characters = "ЄÑホΩѨ을ـگ⌭∰";

bool has_help(const png2dds::args::data& arguments) { return !arguments.error && !arguments.text.empty(); }

bool has_error(const png2dds::args::data& arguments) { return arguments.error && !arguments.text.empty(); }

bool is_valid(const png2dds::args::data& arguments) { return !arguments.error && arguments.text.empty(); }

TEST_CASE("png2dds::arguments input", "[arguments]") {
	const auto input_path = boost::filesystem::temp_directory_path() / utf_characters.data();
	boost::filesystem::create_directories(input_path);

	SECTION("Parse a UTF-8 path") {
		auto arguments = get({binary, input_path.string()});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.input == input_path.string());
	}

	SECTION("Input is read properly after optional arguments.") {
		auto arguments = get({binary, "--verbose", input_path.string()});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.input == input_path.string());
	}

	SECTION("Input is read properly after --.") {
		auto arguments = get({binary, "--overwrite", "--", input_path.string()});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.input == input_path.string());
	}
}

TEST_CASE("png2dds::arguments output", "[arguments]") {
	SECTION("By default, output is not assigned") {
		auto arguments = get({binary, "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(!arguments.output);
	}

	SECTION("Assigning output works as intended.") {
		auto arguments = get({binary, ".", utf_characters});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.output);
		REQUIRE(arguments.output->string() == utf_characters);
	}
}

TEST_CASE("png2dds::arguments optional arguments", "[arguments]") {
	SECTION("Invalid positional arguments are reported as errors") {
		auto arguments = get({binary, "--invalid", "."});
		REQUIRE(has_error(arguments));
		auto shorter = get({binary, "-9", "."});
		REQUIRE(has_error(shorter));
	}

	SECTION("Optional arguments after positional arguments are not parsed") {
		auto arguments = get({binary, ".", "--overwrite"});
		REQUIRE(is_valid(arguments));
		REQUIRE(!arguments.overwrite);
	}

	SECTION("Optional arguments after -- are ignored") {
		auto arguments = get({binary, "--", ".", "--overwrite"});
		REQUIRE(is_valid(arguments));
		REQUIRE(!arguments.overwrite);
	}
}

TEST_CASE("png2dds::arguments help", "[arguments]") {
	SECTION("Show help when called without arguments") {
		const auto arguments = get({binary});
		REQUIRE(has_help(arguments));
	}

	SECTION("Show help when called with the help argument") {
		auto arguments = get({binary, "--help"});
		REQUIRE(has_help(arguments));
		const auto shorter = get({binary, "-h"});
		REQUIRE(has_help(arguments));
	}
}

TEST_CASE("png2dds::arguments format", "[arguments]") {
	SECTION("The default value of format is bc7.") {
		auto arguments = get({binary, "."});
		REQUIRE(arguments.format == png2dds::format::type::bc7);
	}

	SECTION("Parsing bc1.") {
		auto arguments = get({binary, "--format", "bc1", "."});
		REQUIRE(arguments.format == png2dds::format::type::bc1);
	}

	SECTION("Parsing bc1 with alternate case.") {
		auto arguments = get({binary, "-f", "Bc1", "."});
		REQUIRE(arguments.format == png2dds::format::type::bc1);
	}

	SECTION("Parsing bc7.") {
		auto arguments = get({binary, "-f", "bc7", "."});
		REQUIRE(arguments.format == png2dds::format::type::bc7);
	}

	SECTION("Parsing bc1 with alternate case.") {
		auto arguments = get({binary, "--format", "bC7", "."});
		REQUIRE(arguments.format == png2dds::format::type::bc7);
	}
}

TEST_CASE("png2dds::arguments level", "[arguments]") {
	SECTION("The default value of level is 6.") {
		auto arguments = get({binary, "."});
		REQUIRE(arguments.level == png2dds::format::max_level(png2dds::format::type::bc7));
	}

	SECTION("The default value of level when the format is set depends on the format.") {
		const auto arguments_bc1 = get({binary, "-f", png2dds::format::name(png2dds::format::type::bc1), "."});
		REQUIRE(arguments_bc1.level == png2dds::format::max_level(png2dds::format::type::bc1));
		const auto arguments_bc7 = get({binary, "-f", png2dds::format::name(png2dds::format::type::bc7), "."});
		REQUIRE(arguments_bc7.level == png2dds::format::max_level(png2dds::format::type::bc7));
	}

	SECTION("Level is not a number") {
		auto arguments = get({binary, "--level", "not_a_number", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Level is negative") {
		auto arguments = get({binary, "--level", "-4", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Level is out of bounds") {
		auto arguments = get({binary, "--level", "8", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Level is too large to be parsed.") {
		auto arguments = get({binary, "--level", "4444444444444444444444444444444444444444444444444444444444444", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Valid level") {
		constexpr unsigned int level = 5U;
		auto arguments = get({binary, "--level", std::to_string(level), "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.level == level);
		const auto shorter = get({binary, "-l", std::to_string(level), "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.level == level);
	}

	SECTION("Level is larger than the maximum") {
		constexpr auto format_type = png2dds::format::type::bc1;
		const auto arguments = get({binary, "-f", png2dds::format::name(format_type), "-l",
			std::to_string(png2dds::format::max_level(format_type) + 1U), "."});
		REQUIRE(has_error(arguments));
	}
}

TEST_CASE("png2dds::arguments threads", "[arguments]") {
	const auto max_threads = static_cast<std::size_t>(oneapi::tbb::info::default_concurrency());
	SECTION("The default value of threads is determined by the oneTBB library.") {
		auto arguments = get({binary, "."});
		REQUIRE(arguments.threads == max_threads);
	}

	SECTION("Threads is not a number") {
		auto arguments = get({binary, "--threads", "not_a_number", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Threads is negative") {
		auto arguments = get({binary, "--threads", "-4", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Threads is zero") {
		auto arguments = get({binary, "--threads", "0", "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.threads == 1UL);
	}

	SECTION("Threads is larger than the maximum number of threads") {
		auto arguments = get({binary, "--threads", std::to_string(max_threads + 1UL), "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.threads == max_threads);
	}

	SECTION("Threads is too large to be parsed") {
		auto arguments = get({binary, "--threads", "4444444444444444444444444444444444444444444444444444444444444", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Valid number of threads") {
		auto arguments = get({binary, "--threads", std::to_string(max_threads - 1UL), "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.threads == max_threads - 1UL);
		const auto shorter = get({binary, "-th", std::to_string(max_threads - 1UL), "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.threads == max_threads - 1UL);
	}
}

TEST_CASE("png2dds::arguments depth", "[arguments]") {
	SECTION("The default value of threads is the maximum possible value") {
		auto arguments = get({binary, "."});
		REQUIRE(arguments.depth == std::numeric_limits<std::size_t>::max());
	}

	SECTION("Depth is not a number") {
		auto arguments = get({binary, "--depth", "not_a_number", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Depth is negative") {
		auto arguments = get({binary, "--depth", "-4", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Depth is too large to be parsed.") {
		auto arguments = get({binary, "--depth", "4444444444444444444444444444444444444444444444444444444444444", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("A depth of zero implies the maximum possible value.") {
		auto arguments = get({binary, "--depth", "0", "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.depth == std::numeric_limits<std::size_t>::max());
	}

	SECTION("Valid depth") {
		constexpr unsigned int depth = 5U;
		auto arguments = get({binary, "--depth", std::to_string(depth), "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.depth == depth);
		const auto shorter = get({binary, "-d", std::to_string(depth), "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.depth == depth);
	}
}

TEST_CASE("png2dds::arguments overwrite", "[arguments]") {
	SECTION("The default value of overwrite is false") {
		auto arguments = get({binary, "."});
		REQUIRE(!arguments.overwrite);
	}

	SECTION("Providing the overwrite parameter sets its value to true") {
		auto arguments = get({binary, "--overwrite", "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.overwrite);
		const auto shorter = get({binary, "-o", "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.overwrite);
	}
}

TEST_CASE("png2dds::arguments vflip", "[arguments]") {
	SECTION("The default value of vflip is false") {
		auto arguments = get({binary, "."});
		REQUIRE(!arguments.vflip);
	}

	SECTION("Providing the vflip parameter sets its value to true") {
		auto arguments = get({binary, "--vflip", "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.vflip);
		const auto shorter = get({binary, "-vf", "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.vflip);
	}
}

TEST_CASE("png2dds::arguments time", "[arguments]") {
	SECTION("The default value of time is false") {
		auto arguments = get({binary, "."});
		REQUIRE(!arguments.time);
	}

	SECTION("Providing the time parameter sets its value to true") {
		auto arguments = get({binary, "--time", "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.time);
		const auto shorter = get({binary, "-t", "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.time);
	}
}

TEST_CASE("png2dds::arguments verbose", "[arguments]") {
	SECTION("The default value of verbose is false") {
		auto arguments = get({binary, "."});
		REQUIRE(!arguments.verbose);
	}

	SECTION("Providing the verbose parameter sets its value to true") {
		auto arguments = get({binary, "--verbose", "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.verbose);
		const auto shorter = get({binary, "-v", "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.verbose);
	}
}

TEST_CASE("png2dds::arguments regex", "[arguments]") {
	SECTION("By default regex is empty") {
		auto arguments = get({binary, "."});
		REQUIRE(arguments.regex.error().empty());
		REQUIRE(arguments.regex.database() == nullptr);
		REQUIRE(arguments.regex.allocate_scratch().get() == nullptr);
	}

	SECTION("Compilation errors are reported. The regex is empty") {
		auto arguments = get({binary, "--regex", "(()", "."});
		REQUIRE(has_error(arguments));
		REQUIRE(!arguments.regex.error().empty());
		REQUIRE(arguments.regex.database() == nullptr);
		REQUIRE(arguments.regex.allocate_scratch().get() == nullptr);
	}

	SECTION("Compiled regular expression") {
		auto arguments = get({binary, "--regex", utf_characters, "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.regex.error().empty());
		REQUIRE(arguments.regex.database() != nullptr);
		REQUIRE(arguments.regex.allocate_scratch().get() != nullptr);
		const auto shorter = get({binary, "-r", utf_characters, "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.regex.error().empty());
		REQUIRE(shorter.regex.database() != nullptr);
		REQUIRE(shorter.regex.allocate_scratch().get() != nullptr);
	}
}