/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "todds/arguments.hpp"
#include "todds/format.hpp"
#include "todds/project.hpp"

#include <boost/filesystem/operations.hpp>
#include <oneapi/tbb/info.h>

#include <limits>

#include <catch2/catch_test_macros.hpp>

using todds::args::get;

constexpr auto binary = todds::project::name();

constexpr std::string_view utf_characters = "ЄÑホΩѨ을ـگ⌭∰";

bool has_help(const todds::args::data& arguments) { return !arguments.error && !arguments.text.empty(); }

bool has_error(const todds::args::data& arguments) { return arguments.error && !arguments.text.empty(); }

bool is_valid(const todds::args::data& arguments) { return !arguments.error && arguments.text.empty(); }

TEST_CASE("todds::arguments input", "[arguments]") {
	const auto input_path = boost::filesystem::temp_directory_path() / utf_characters.data();
	boost::filesystem::create_directories(input_path);

	SECTION("Parse a UTF-8 path") {
		const auto arguments = get({binary, input_path.string()});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.input == input_path.string());
	}

	SECTION("Input is read properly after optional arguments.") {
		const auto arguments = get({binary, "--verbose", input_path.string()});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.input == input_path.string());
	}

	SECTION("Input is read properly after --.") {
		const auto arguments = get({binary, "--overwrite", "--", input_path.string()});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.input == input_path.string());
	}
}

TEST_CASE("todds::arguments output", "[arguments]") {
	SECTION("By default, output is not assigned") {
		const auto arguments = get({binary, "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(!arguments.output);
	}

	SECTION("Assigning output works as intended.") {
		const auto arguments = get({binary, ".", utf_characters});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.output.has_value());
		const bool assignment_working = arguments.output.has_value() && arguments.output->string() == utf_characters;
		REQUIRE(assignment_working);
	}
}

TEST_CASE("todds::arguments optional arguments", "[arguments]") {
	SECTION("Invalid positional arguments are reported as errors") {
		const auto arguments = get({binary, "--invalid", "."});
		REQUIRE(has_error(arguments));
		const auto shorter = get({binary, "-9", "."});
		REQUIRE(has_error(shorter));
	}

	SECTION("Optional arguments after positional arguments are not parsed") {
		const auto arguments = get({binary, ".", "--overwrite"});
		REQUIRE(is_valid(arguments));
		REQUIRE(!arguments.overwrite);
	}

	SECTION("Optional arguments after -- are ignored") {
		const auto arguments = get({binary, "--", ".", "--overwrite"});
		REQUIRE(is_valid(arguments));
		REQUIRE(!arguments.overwrite);
	}
}

TEST_CASE("todds::arguments help", "[arguments]") {
	SECTION("Show help when called without arguments") {
		const auto arguments = get({binary});
		REQUIRE(has_help(arguments));
	}

	SECTION("Show help when called with the help argument") {
		const auto arguments = get({binary, "--help"});
		REQUIRE(has_help(arguments));
		const auto shorter = get({binary, "-h"});
		REQUIRE(has_help(arguments));
	}
}

TEST_CASE("todds::arguments format", "[arguments]") {
	SECTION("The default value of format is bc7.") {
		const auto arguments = get({binary, "."});
		REQUIRE(arguments.format == todds::format::type::bc7);
	}

	SECTION("Parsing bc1.") {
		const auto arguments = get({binary, "--format", "bc1", "."});
		REQUIRE(!has_error(arguments));
		REQUIRE(arguments.format == todds::format::type::bc1);
	}

	SECTION("Parsing bc1 with alternate case.") {
		const auto arguments = get({binary, "-f", "Bc1", "."});
		REQUIRE(!has_error(arguments));
		REQUIRE(arguments.format == todds::format::type::bc1);
	}

	SECTION("Parsing bc7.") {
		const auto arguments = get({binary, "-f", "bc7", "."});
		REQUIRE(!has_error(arguments));
		REQUIRE(arguments.format == todds::format::type::bc7);
	}

	SECTION("Parsing bc7 with alternate case.") {
		const auto arguments = get({binary, "--format", "bC7", "."});
		REQUIRE(!has_error(arguments));
		REQUIRE(arguments.format == todds::format::type::bc7);
	}
}

TEST_CASE("todds::arguments quality", "[arguments]") {
	SECTION("The default quality is set to really_slow.") {
		const auto arguments = get({binary, "."});
		REQUIRE(arguments.quality == todds::format::quality::really_slow);
		const auto arguments_bc1 = get({binary, "-f", todds::format::name(todds::format::type::bc1), "."});
		REQUIRE(arguments_bc1.quality == todds::format::quality::really_slow);
		const auto arguments_bc7 = get({binary, "-f", todds::format::name(todds::format::type::bc7), "."});
		REQUIRE(arguments_bc7.quality == todds::format::quality::really_slow);
	}

	SECTION("Error when the quality is not a number") {
		const auto arguments = get({binary, "--quality", "not_a_number", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Error when the quality is negative") {
		const auto arguments = get({binary, "--quality", "-4", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Quality is out of bounds") {
		const auto arguments =
			get({binary, "--quality", std::to_string(static_cast<int>(todds::format::quality::maximum) + 1), "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Level is too large to be parsed.") {
		const auto arguments =
			get({binary, "--quality", "4444444444444444444444444444444444444444444444444444444444444", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Valid level") {
		constexpr unsigned int level = 5U;
		constexpr std::string_view level_str = "5";
		const auto arguments = get({binary, "--quality", level_str, "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(static_cast<unsigned int>(arguments.quality) == level);
		const auto shorter = get({binary, "-q", level_str, "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(static_cast<unsigned int>(shorter.quality) == level);
	}
}

TEST_CASE("todds::arguments mipmaps", "[arguments]") {
	SECTION("The default value of mipmaps is true") {
		const auto arguments = get({binary, "."});
		REQUIRE(arguments.mipmaps);
	}

	SECTION("Providing the mipmaps parameter sets its value to false") {
		const auto arguments = get({binary, "--no-mipmaps", "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(!arguments.mipmaps);
		const auto shorter = get({binary, "-nm", "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(!shorter.mipmaps);
	}
}

TEST_CASE("todds::arguments fix_size", "[arguments]") {
	SECTION("The default value of fix_size is false") {
		const auto arguments = get({binary, "."});
		REQUIRE(!arguments.fix_size);
	}

	SECTION("Providing the fix_size parameter sets its value to true") {
		const auto arguments = get({binary, "--fix_size", "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.fix_size);
		const auto shorter = get({binary, "-fs", "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.fix_size);
	}
}

TEST_CASE("todds::arguments mipmap_filter", "[arguments]") {
	SECTION("The default value of mipmap_filter is lanczos.") {
		const auto arguments = get({binary, "."});
		REQUIRE(!has_error(arguments));
		REQUIRE(arguments.mipmap_filter == todds::filter::type::lanczos);
	}

	SECTION("Parsing nearest.") {
		const auto arguments = get({binary, "--mipmap_filter", "nearest", "."});
		REQUIRE(!has_error(arguments));
		REQUIRE(arguments.mipmap_filter == todds::filter::type::nearest);
	}

	SECTION("Parsing nearest with alternate case.") {
		const auto arguments = get({binary, "-mf", "nEArEst", "."});
		REQUIRE(!has_error(arguments));
		REQUIRE(arguments.mipmap_filter == todds::filter::type::nearest);
	}
}

TEST_CASE("todds::arguments mipmap_blur", "[arguments]") {
	SECTION("The default value of mipmap_blur is between 0 and 1.") {
		const auto arguments = get({binary, "."});
		REQUIRE(arguments.mipmap_blur > 0.0);
		REQUIRE(arguments.mipmap_blur <= 1.0);
	}

	SECTION("mipmap_blur is not a number") {
		const auto arguments = get({binary, "--mipmap-blur", "not_a_number", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("mipmap_blur is negative") {
		const auto arguments = get({binary, "--mipmap-blur", "-4.7", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("mipmap_blur is zero") {
		const auto arguments = get({binary, "--mipmap-blur", "0.0", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Valid mipmap_blur value") {
		const auto arguments = get({binary, "--mipmap-blur", std::to_string(0.1), "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.mipmap_blur == 0.1);
		const auto shorter = get({binary, "-mb", std::to_string(0.1), "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.mipmap_blur == 0.1);
	}
}

TEST_CASE("todds::arguments scale", "[arguments]") {
	SECTION("The default value of scale is 100%.") {
		const auto arguments = get({binary, "."});
		REQUIRE(arguments.scale == 100U);
	}

	SECTION("Scale is not a number") {
		const auto arguments = get({binary, "--scale", "not_a_number", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Scale is negative") {
		const auto arguments = get({binary, "--scale", "-4", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Scale is zero") {
		const auto arguments = get({binary, "--scale", "0", "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.scale == 1UL);
	}

	SECTION("Scale is too large.") {
		const auto arguments = get({binary, "--scale", std::to_string(5000U), "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.scale == 1000U);
	}

	SECTION("Scale is too large to be parsed") {
		const auto arguments =
			get({binary, "--scale", "4444444444444444444444444444444444444444444444444444444444444", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Valid scale value") {
		const auto arguments = get({binary, "--scale", std::to_string(150U), "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.scale == 150U);
		const auto shorter = get({binary, "-sc", std::to_string(150U), "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.scale == 150U);
	}
}

TEST_CASE("todds::arguments max_size", "[arguments]") {
	SECTION("The default value of max_size is 0, which means default.") {
		const auto arguments = get({binary, "."});
		REQUIRE(arguments.max_size == 0U);
	}

	SECTION("max_size is not a number") {
		const auto arguments = get({binary, "--max-size", "not_a_number", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("max_size is negative") {
		const auto arguments = get({binary, "--max-size", "-4", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("max_size is too large to be parsed") {
		const auto arguments =
			get({binary, "--max-size", "4444444444444444444444444444444444444444444444444444444444444", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Valid max size value") {
		const auto arguments = get({binary, "--max-size", std::to_string(150U), "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.max_size == 150U);
		const auto shorter = get({binary, "-ms", std::to_string(150U), "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.max_size == 150U);
	}
}

TEST_CASE("todds::arguments scale_filter", "[arguments]") {
	SECTION("The default value of scale_filter is lanczos.") {
		const auto arguments = get({binary, "."});
		REQUIRE(!has_error(arguments));
		REQUIRE(arguments.scale_filter == todds::filter::type::lanczos);
	}

	SECTION("Parsing nearest.") {
		const auto arguments = get({binary, "--scale-filter", "nearest", "."});
		REQUIRE(!has_error(arguments));
		REQUIRE(arguments.scale_filter == todds::filter::type::nearest);
	}

	SECTION("Parsing nearest with alternate case.") {
		const auto arguments = get({binary, "-sf", "nEArEst", "."});
		REQUIRE(!has_error(arguments));
		REQUIRE(arguments.scale_filter == todds::filter::type::nearest);
	}
}

TEST_CASE("todds::arguments threads", "[arguments]") {
	const auto max_threads = static_cast<std::size_t>(oneapi::tbb::info::default_concurrency());
	SECTION("The default value of threads is determined by the oneTBB library.") {
		const auto arguments = get({binary, "."});
		REQUIRE(arguments.threads == max_threads);
	}

	SECTION("Threads is not a number") {
		const auto arguments = get({binary, "--threads", "not_a_number", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Threads is negative") {
		const auto arguments = get({binary, "--threads", "-4", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Threads is zero") {
		const auto arguments = get({binary, "--threads", "0", "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.threads == 1UL);
	}

	SECTION("Threads is larger than the maximum number of threads") {
		const auto arguments = get({binary, "--threads", std::to_string(max_threads + 1UL), "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.threads == max_threads);
	}

	SECTION("Threads is too large to be parsed") {
		const auto arguments =
			get({binary, "--threads", "4444444444444444444444444444444444444444444444444444444444444", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Valid number of threads") {
		const auto arguments = get({binary, "--threads", std::to_string(max_threads - 1UL), "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.threads == max_threads - 1UL);
		const auto shorter = get({binary, "-th", std::to_string(max_threads - 1UL), "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.threads == max_threads - 1UL);
	}
}

TEST_CASE("todds::arguments depth", "[arguments]") {
	SECTION("The default value of depth is the maximum possible value") {
		const auto arguments = get({binary, "."});
		const auto value = std::numeric_limits<std::size_t>::max();
		REQUIRE(arguments.depth == value);
	}

	SECTION("Depth is not a number") {
		const auto arguments = get({binary, "--depth", "not_a_number", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Depth is negative") {
		const auto arguments = get({binary, "--depth", "-4", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("Depth is too large to be parsed.") {
		const auto arguments =
			get({binary, "--depth", "4444444444444444444444444444444444444444444444444444444444444", "."});
		REQUIRE(has_error(arguments));
	}

	SECTION("A depth of zero implies the maximum possible value.") {
		const auto arguments = get({binary, "--depth", "0", "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.depth == std::numeric_limits<std::size_t>::max());
	}

	SECTION("Valid depth") {
		constexpr unsigned int depth = 5U;
		const auto arguments = get({binary, "--depth", std::to_string(depth), "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.depth == depth);
		const auto shorter = get({binary, "-d", std::to_string(depth), "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.depth == depth);
	}
}

TEST_CASE("todds::arguments overwrite", "[arguments]") {
	SECTION("The default value of overwrite is false") {
		const auto arguments = get({binary, "."});
		REQUIRE(!arguments.overwrite);
	}

	SECTION("Providing the overwrite parameter sets its value to true") {
		const auto arguments = get({binary, "--overwrite", "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.overwrite);
		const auto shorter = get({binary, "-o", "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.overwrite);
	}
}

TEST_CASE("todds::arguments overwrite_new", "[arguments]") {
	SECTION("The default value of overwrite is false") {
		const auto arguments = get({binary, "."});
		REQUIRE(!arguments.overwrite_new);
	}

	SECTION("Providing the overwrite_new parameter sets its value to true") {
		const auto arguments = get({binary, "--overwrite_new", "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.overwrite_new);
		const auto shorter = get({binary, "-on", "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.overwrite_new);
	}

	SECTION("Setting overwrite and overwrite_new at the same time triggers an error.") {
		const auto arguments = get({binary, "--overwrite", "--overwrite_new", "."});
		REQUIRE(has_error(arguments));
	}
}

TEST_CASE("todds::arguments vflip", "[arguments]") {
	SECTION("The default value of vflip is false") {
		const auto arguments = get({binary, "."});
		REQUIRE(!arguments.vflip);
	}

	SECTION("Providing the vflip parameter sets its value to true") {
		const auto arguments = get({binary, "--vflip", "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.vflip);
		const auto shorter = get({binary, "-vf", "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.vflip);
	}
}

TEST_CASE("todds::arguments time", "[arguments]") {
	SECTION("The default value of time is false") {
		const auto arguments = get({binary, "."});
		REQUIRE(!arguments.time);
	}

	SECTION("Providing the time parameter sets its value to true") {
		const auto arguments = get({binary, "--time", "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.time);
		const auto shorter = get({binary, "-t", "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.time);
	}
}

TEST_CASE("todds::arguments verbose", "[arguments]") {
	SECTION("The default value of verbose is false") {
		const auto arguments = get({binary, "."});
		REQUIRE(!arguments.verbose);
	}

	SECTION("Providing the verbose parameter sets its value to true") {
		const auto arguments = get({binary, "--verbose", "."});
		REQUIRE(is_valid(arguments));
		REQUIRE(arguments.verbose);
		const auto shorter = get({binary, "-v", "."});
		REQUIRE(is_valid(shorter));
		REQUIRE(shorter.verbose);
	}
}

TEST_CASE("todds::arguments regex", "[arguments]") {
	SECTION("By default regex is empty") {
		const auto arguments = get({binary, "."});
		REQUIRE(arguments.regex.error().empty());
		REQUIRE(arguments.regex.database() == nullptr);
		REQUIRE(arguments.regex.allocate_scratch().get() == nullptr);
	}

	SECTION("Compilation errors are reported. The regex is empty") {
		const auto arguments = get({binary, "--regex", "(()", "."});
		REQUIRE(has_error(arguments));
		REQUIRE(!arguments.regex.error().empty());
		REQUIRE(arguments.regex.database() == nullptr);
		REQUIRE(arguments.regex.allocate_scratch().get() == nullptr);
	}

	SECTION("Compiled regular expression") {
		const auto arguments = get({binary, "--regex", utf_characters, "."});
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