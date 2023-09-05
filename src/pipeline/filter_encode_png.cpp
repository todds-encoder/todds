/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "filter_encode_png.hpp"

#include "todds/png.hpp"
#include "todds/profiler.hpp"

#include <fmt/format.h>

#include "filter_common.hpp"

namespace todds::pipeline::impl {

class encode_png_image final {
public:
	explicit encode_png_image(const paths_vector& paths, error_queue& errors)
		: _paths{paths}
		, _errors{errors} {}

	png_data operator()(std::unique_ptr<mipmap_image> input) const {
		TracyZoneScopedN("encode_png");
		if (input == nullptr || input->file_index() == error_file_index) [[unlikely]] {
			png_data error{};
			error.file_index = error_file_index;
			return error;
		}

		TracyZoneFileIndex(input->file_index());
		const string& path = _paths[input->file_index()].first.string();
		try {
			png_data result;
			result.file_index = input->file_index();
			result.image = png::encode(path, std::move(input));
			return result;
		} catch (const std::runtime_error& exc) {
			_errors.push(fmt::format("PNG Encoding error {:s} -> {:s}", path, exc.what()));
		}

		return {};
	}

private:
	const paths_vector& _paths;
	error_queue& _errors;
};

oneapi::tbb::filter<std::unique_ptr<mipmap_image>, png_data> encode_png_filter(
	const paths_vector& paths, error_queue& errors) {
	return make_filter<std::unique_ptr<mipmap_image>, png_data>(
		tbb::filter_mode::parallel, encode_png_image{paths, errors});
}

} // namespace todds::pipeline::impl
