/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "filter_load_png.hpp"

#include "todds/profiler.hpp"

#include <boost/nowide/fstream.hpp>
#include <boost/predef.h>
#include <fmt/format.h>

#if defined(TODDS_PIPELINE_DUMP)
#include <boost/dll/runtime_symbol_info.hpp>
#endif // defined(TODDS_PIPELINE_DUMP)

namespace todds::pipeline::impl {

class load_png_file final {
public:
	explicit load_png_file(const paths_vector& paths, std::atomic<std::size_t>& counter, std::atomic<bool>& force_finish,
		report_queue& updates) noexcept
		: _paths{paths}
		, _counter{counter}
		, _force_finish{force_finish}
		, _updates{updates} {}

	png_file operator()(oneapi::tbb::flow_control& flow) const {
		const std::size_t index = _counter++;
		TracyZoneScopedN("load");
		TracyZoneFileIndex(index);

		if (index >= _paths.size() || _force_finish) [[unlikely]] {
			flow.stop();
			return {};
		}

#if BOOST_OS_WINDOWS
		const boost::filesystem::path input{R"(\\?\)" + _paths[index].first.string()};
#else
		const boost::filesystem::path& input{_paths[index].first};
#endif
		boost::nowide::ifstream ifs{input, std::ios::in | std::ios::binary};

		if (!ifs.is_open()) [[unlikely]] {
			_updates.emplace(
				report_type::pipeline_error, fmt::format("Load PNG file error in {:s}", _paths[index].first.string()));
		}

		png_file result{{std::istreambuf_iterator<char>{ifs}, {}}, index};

		if (result.buffer.empty()) [[unlikely]] {
			_updates.emplace(report_type::pipeline_error,
				fmt::format("Could not load any data for PNG file {:s}", _paths[index].first.string()));
		}
#if defined(TODDS_PIPELINE_DUMP)
		else {
			const auto dmp_path = boost::dll::program_location().parent_path() / "load_png.dmp";
			boost::nowide::ofstream dmp{dmp_path, std::ios::out | std::ios::binary};
			const std::uint8_t* file_start = result.buffer.data();
			dmp.write(reinterpret_cast<const char*>(file_start), static_cast<std::ptrdiff_t>(result.buffer.size()));
		}
#endif // defined(TODDS_PIPELINE_DUMP)

		return result;
	}

private:
	const paths_vector& _paths;
	std::atomic<std::size_t>& _counter;
	std::atomic<bool>& _force_finish;
	report_queue& _updates;
};

oneapi::tbb::filter<void, png_file> load_png_filter(const paths_vector& paths, std::atomic<std::size_t>& counter,
	std::atomic<bool>& force_finish, report_queue& updates) {
	return oneapi::tbb::make_filter<void, png_file>(
		oneapi::tbb::filter_mode::parallel, load_png_file(paths, counter, force_finish, updates));
}
} // namespace todds::pipeline::impl
