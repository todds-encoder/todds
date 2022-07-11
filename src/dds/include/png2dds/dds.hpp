/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PNG2DDS_DDS_HPP
#define PNG2DDS_DDS_HPP

#include "png2dds/dds_image.hpp"
#include "png2dds/image.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ispc {
// Forward-declaration of the internal bc7e object required by the encoder.
struct bc7e_compress_block_params;
} // namespace ispc

namespace png2dds::dds {

class encoder final {
public:
	explicit encoder(unsigned int level);
	encoder(const encoder&) = delete;
	encoder(encoder&&) = default;
	encoder& operator=(const encoder&) = delete;
	encoder& operator=(encoder&&) = default;
	~encoder();
	[[nodiscard]] dds_image encode(const image& png) const;

private:
	std::unique_ptr<ispc::bc7e_compress_block_params> _pimpl;
};

} // namespace png2dds::dds

#endif // PNG2DDS_DDS_HPP
