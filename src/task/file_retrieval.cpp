/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "todds/file_retrieval.hpp"

#include <boost/nowide/convert.hpp>
#include <boost/nowide/fstream.hpp>
#include <boost/predef.h>
#include <fmt/format.h>

#include <cwctype>
#include <stack>
#include <string_view>
#include <utility>

namespace fs = boost::filesystem;
using todds::pipeline::paths_vector;

using path_view = std::basic_string_view<fs::path::value_type>;
using path_string = fs::path::string_type;
using path_char = path_string::value_type;
using path_diff_t = path_string::difference_type;

#if BOOST_OS_WINDOWS

#define PATH_LITERAL_HELPER(literal) L##literal
#define PATH_LITERAL(literal) PATH_LITERAL_HELPER(literal)
#define PATH_STRING_WIDEN(str) boost::nowide::widen(str)
#define PATH_STRING_NARROW(str) boost::nowide::narrow(str)

#else

#define PATH_LITERAL(x) x
#define PATH_STRING_WIDEN(str) str
#define PATH_STRING_NARROW(str) str

#endif // BOOST_OS_WINDOWS

constexpr path_diff_t extension_size = 4;

constexpr path_view dds_extension{PATH_LITERAL(".dds")};
static_assert(static_cast<path_diff_t>(dds_extension.size()) == extension_size);

constexpr path_view png_extension{PATH_LITERAL(".png")};
static_assert(static_cast<path_diff_t>(png_extension.size()) == extension_size);

constexpr path_view txt_extension{PATH_LITERAL(".txt")};
static_assert(static_cast<path_diff_t>(txt_extension.size()) == extension_size);

// lhs may have mixed case. rhs is assumed to be lower-case.
template<typename char_type> bool insensitive_equals(const path_char lhs, const path_char rhs) {
	if constexpr (std::is_same_v<char_type, wchar_t>) {
		return std::towlower(lhs) == rhs; // NOLINT
	} else {
		return std::tolower(lhs) == rhs;
	}
}

// lhs may have mixed case. rhs is assumed to be lower-case.
bool insensitive_equals(const path_view lhs, const path_view rhs) {
	return lhs.size() == rhs.size() && std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin(), insensitive_equals<path_char>);
}

path_view extension_view(const fs::path& path) {
	if (!path.has_extension()) { return {}; }
	const path_string& path_str = path.native();
	const auto offset = static_cast<path_diff_t>(path_str.size()) - extension_size;
	if (offset <= 0) { return {}; }
	return {path_str.cbegin() + offset, path_str.cend()};
}

bool has_extension(const fs::path& path, const path_view extension) {
	return insensitive_equals(extension_view(path), extension);
}

class file_retrieval_state final {
public:
	file_retrieval_state(todds::report_queue& updates, const todds::vector<boost::filesystem::path>& input,
		std::optional<boost::filesystem::path> output, todds::format::type format, bool create_folders, bool overwrite,
		bool overwrite_new, const todds::string& substring, const todds::regex& regex, const std::size_t depth) // NOLINT
		: _updates{updates}
		, _input{input}
		, _output{std::move(output)}
		, _output_extension{format == todds::format::type::png ? png_extension : dds_extension}
		, _create_folders{create_folders}
		, _overwrite{overwrite}
		, _overwrite_new{overwrite_new && !_overwrite}
		, _substring{PATH_STRING_WIDEN(substring)}
		, _regex{regex}
		, _depth{depth}
		, _files{} {}

	paths_vector get_result() {
		process_user_input();
		paths_vector result{};
		std::swap(result, _files);
		return result;
	}

private:
	[[nodiscard]] bool path_matches_criteria(const fs::path& path) const {
		return (!_substring.empty() && path.native().find(_substring) != std::string::npos) ||
					 (_regex.valid() && _regex.match(PATH_STRING_NARROW(path.native())));
	}

	[[nodiscard]] bool should_generate(const fs::path& input_path, const fs::path& output_path) const {
		return input_path != output_path &&
					 (_overwrite || !fs::exists(output_path) ||
						 (_overwrite_new && (fs::last_write_time(input_path) > fs::last_write_time(output_path))));
	}

	void process_user_input() {
		for (const fs::path& path : _input) {
			if (fs::is_directory(path)) {
				process_user_input_directory(path);
			} else if (!has_extension(path, png_extension) || !process_file(path, path.parent_path())) {
				_updates.emplace(
					todds::report_type::pipeline_error, fmt::format("{:s} is not a PNG file or a directory.", path.string()));
			}
		}
	}

	void process_user_input_directory(const fs::path& path) {
		const fs::directory_entry dir{path};

		std::size_t match_update_depth = 0U;
		bool current_match = path_matches_criteria(path);

		for (fs::recursive_directory_iterator itr{dir}; itr != fs::recursive_directory_iterator{}; ++itr) {
			// When current_match is true, there is no need to update when going deeper.
			// When reducing depth, the value must always be updated.
			if (itr->is_directory()) {
				const auto current_depth = static_cast<size_t>(itr.depth());
				if (((current_depth > match_update_depth && !current_match) || current_depth < match_update_depth)) {
					current_match = path_matches_criteria(path);
					match_update_depth = current_depth;
				}
			}

			_updates.emplace(todds::report_type::retrieving_files_progress);

			try {
				const fs::path& current_path = itr->path();
				if (has_extension(current_path, png_extension)) {
					// Determine output path.
					fs::path current_output{};
					if (_output.has_value()) {
						current_output = _output.value();
						const auto relative = fs::relative(current_path.parent_path(), path);
						if (!relative.filename_is_dot()) { current_output /= relative; }
						// Create the output folder if necessary.
						if (_create_folders && !fs::exists(current_output)) { fs::create_directories(current_output); }
					} else {
						current_output = current_path.parent_path();
					}

					process_file(current_path, current_output, current_match);
				}
			} catch (const fs::filesystem_error& error) {
				_updates.emplace(todds::report_type::pipeline_error, error.what());
			}

			if (static_cast<unsigned int>(itr.depth()) >= _depth) { itr.disable_recursion_pending(); }
		}
	}

	// Assumes that the extension check has been performed already.
	bool process_file(const fs::path& input_file, const fs::path& output_path, bool previous_match = false) {
		if (!previous_match && !path_matches_criteria(input_file)) { return false; }
		const fs::path output_file = (output_path / input_file.stem()) += _output_extension.data();
		if (!should_generate(input_file, output_file)) { return false; }
		_files.emplace_back(input_file, output_file);
		return true;
	}

	// Error reporting
	todds::report_queue& _updates;
	// Input and output.
	const todds::vector<boost::filesystem::path>& _input;
	std::optional<boost::filesystem::path> _output;
	const path_view _output_extension;
	const bool _create_folders;
	// File validation parameters.
	const bool _overwrite;
	const bool _overwrite_new;
	const path_string _substring;
	const todds::regex& _regex;
	const std::size_t _depth;
	// Input state parameters.
	paths_vector _files;
};

file_retrieval_state from_args(const todds::args::data& args, todds::report_queue& updates) {
	const bool has_output = args.output.has_value();
	const bool create_folders = has_output && !args.dry_run && !args.clean;

	if (!args.input.empty() && has_extension(args.input[0], txt_extension)) {
		if (args.input.size() > 1U) {
			updates.emplace(todds::report_type::pipeline_error, "TXT file processing only supports a single input.");
		}
		if (has_output) {
			updates.emplace(
				todds::report_type::pipeline_error, "Output argument is not supported when processing a TXT file.");
		}
		todds::vector<boost::filesystem::path> input{};
		boost::nowide::fstream stream{args.input[0]};
		todds::string buffer;
		while (std::getline(stream, buffer)) { input.push_back(fs::canonical(fs::path{buffer})); }
		return {updates, input, std::optional<boost::filesystem::path>{}, args.format, create_folders, args.overwrite,
			args.overwrite_new, args.substring, args.regex, args.depth};
	}

	std::optional<boost::filesystem::path> output = args.output;
	if (args.input.size() > 1U && has_output) {
		updates.emplace(
			todds::report_type::pipeline_error, "Output argument is not supported when processing more than one input.");
		output.reset();
	}

	return {updates, args.input, std::move(output), args.format, create_folders, args.overwrite, args.overwrite_new,
		args.substring, args.regex, args.depth};
}

namespace todds {

paths_vector get_paths(const todds::args::data& arguments, todds::report_queue& updates) {
	file_retrieval_state state = from_args(arguments, updates);
	return state.get_result();
}

} // namespace todds
