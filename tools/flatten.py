# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

import argparse
import os
import pathlib
import shutil
import sys


def get_parsed_args():
    parser = argparse.ArgumentParser(
        description='Copy all PNG files from the input (including those in subdirectories) to the output folder')
    parser.add_argument('input', metavar='input', type=str,
                        help='Directory containing multiple directories with PNGs to be used for testing')
    parser.add_argument('output', metavar='output', type=str, help='Path to which the PNGs will be copied.')
    return parser.parse_args()


def validate_args(arguments):
    if not os.path.isdir(arguments.input):
        return f'Input directory {arguments.input} is not valid'

    if os.path.exists(args.output) and not os.path.isdir(arguments.output):
        return f'{args.output} is not a directory'

    return ''


if __name__ == '__main__':
    # Argument parsing and validation.
    args = get_parsed_args()
    args_error = validate_args(args)
    if len(args_error) > 0:
        sys.exit(args_error)

    if not os.path.exists(args.output):
        pathlib.Path(args.output).mkdir(parents=True, exist_ok=True)

    for dirpath, _, filenames in os.walk(args.input, topdown=False):
        for filename in filenames:
            file_parts = os.path.splitext(os.path.basename(filename))
            if file_parts[1].lower() != '.png':
                continue
            source = os.path.join(dirpath, filename)
            target = os.path.join(args.output, filename)

            index = 0
            while os.path.exists(target):
                index += 1
                target = os.path.join(args.output, f'{file_parts[0]}_{index}_{file_parts[1]}')

            shutil.copy(source, target)
