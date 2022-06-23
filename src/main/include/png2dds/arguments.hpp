/*
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
 * distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef PNG2DDS_ARGUMENTS_HPP
#define PNG2DDS_ARGUMENTS_HPP

namespace argparse {
class ArgumentParser;
} // namespace argparse

namespace png2dds {
/**
 * Wraps the definition of png2dds command-line arguments.
 * @return Argument parser instance.
 */
argparse::ArgumentParser arguments();
} // namespace png2dds

#endif // PNG2DDS_ARGUMENTS_HPP
