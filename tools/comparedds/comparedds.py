# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

# Since some tools evaluated by this script only work on Windows, the script also requires Windows.

# Expects the following executables to be in the PATH:
# * nvbatchcompress.exe (NVIDIA Texture Tools): DDS encoding tool
# * texconv.exe (DirectXTex): DDS encoding tool
# * bc7enc.exe (bc7enc_rdo): DDS encoding tool
# * png2dds.exe: DDS encoding tool
# * flip.exe (NVIDIA FLIP Difference Evaluator): FLIP metric
# * magick.exe (ImageMagick): PSNR, RMSE and SSIM metrics

import argparse
import collections
import csv
import os
import shutil
import subprocess
import sys
import winreg

# Tools used by this script
bc7enc_tool = 'bc7enc'
nvtt_tool = 'nvtt'
png2dds_tool = 'png2dds'
texconv_tool = 'texconv'
flip_executable = 'flip'
magick_executable = 'magick'

EncoderData = collections.namedtuple('EncoderData', 'executable params')

# Maps tool arguments to their executable and parameters.
encoder_data = {
    bc7enc_tool: EncoderData('bc7enc', ('-q', '-g', '-u6')),
    nvtt_tool: EncoderData('nvbatchcompress', ('-highest', '-bc7', '-silent')),
    png2dds_tool: EncoderData('png2dds', ('-o',)),
    texconv_tool: EncoderData('texconv', ('-y', '-f', 'BC7_UNORM', '-bc', 'x')),
}


def get_parsed_args():
    parser = argparse.ArgumentParser(description='Compare DDS encoders.')
    for tool, (executable, _) in encoder_data.items():
        parser.add_argument(f'--{tool}', action='store_true',
                            help=f'Compare {tool}. The {executable} executable must be in the PATH.')
    parser.add_argument(f'--info', action='store_true', help=f'Generate a CSV with system and tool information')
    parser.add_argument('input', metavar='input', type=str, help='Path containing PNGs to be used for testing')
    parser.add_argument('output', metavar='output', type=str, help='Path in which output PNGs will be created.')
    return parser.parse_args()


def validate_args(arguments):
    for tool, (executable, _) in encoder_data.items():
        if getattr(arguments, tool) and shutil.which(executable) is None:
            return f'To use {tool}, {executable} must be present in the PATH'

    if not os.path.isdir(arguments.input):
        return f'Input directory {arguments.input} is not valid'

    if os.path.exists(args.output) and not os.path.isdir(arguments.output):
        return f'{args.output} is not a directory'

    return ''


def bc7enc_version():
    output = subprocess.run([encoder_data[bc7enc_tool].executable, ], text=True, check=False, stdout=subprocess.PIPE,
                            stderr=subprocess.DEVNULL).stdout
    _, output = output.split('bc7enc ', 1)
    output, _ = output.split(' - ', 1)
    return output


def nvtt_version():
    output = subprocess.run([encoder_data[nvtt_tool].executable, ], text=True, check=False, stdout=subprocess.PIPE,
                            stderr=subprocess.DEVNULL).stdout
    output, _ = output.split(' - ', 1)
    _, output = output.split('Tools ', 1)
    return output


def png2dds_version():
    output = subprocess.run([encoder_data[png2dds_tool].executable, '--help'], text=True, check=False,
                            stdout=subprocess.PIPE,
                            stderr=subprocess.DEVNULL).stdout
    _, output = output.split('png2dds ', 1)
    output, _ = output.split('\n', 1)
    return output.strip()


def texconv_version():
    output = subprocess.check_output([encoder_data[texconv_tool].executable, ]).decode('utf-8')
    _, output = output.split('Version ', 1)
    output, _ = output.split('\n', 1)
    return output.strip()


def flip_version():
    output = subprocess.run([flip_executable, ], text=True, check=False, stdout=subprocess.PIPE,
                            stderr=subprocess.DEVNULL).stdout
    _, output = output.split('FLIP ', 1)
    output, _ = output.split('.\n', 1)
    return output.strip()


def magick_version():
    output = subprocess.check_output([magick_executable, '-version']).decode('utf-8')
    _, output = output.split('ImageMagick ', 1)
    output, _ = output.split(' ', 1)
    return output.strip()


def generate_info(arguments):
    with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, r'Hardware\Description\System\CentralProcessor\0') as key:
        cpu_name = winreg.QueryValueEx(key, 'ProcessorNameString')[0].strip()
    gpu_name = 'Unknown'
    try:
        output = subprocess.check_output(['nvidia-smi', '-L']).decode('utf-8')
        _, output = output.split(': ', 1)
        gpu_name, _ = output.split(' (', 1)
    except FileNotFoundError:
        pass

    csv_out = csv.writer(sys.stdout)
    csv_out.writerow(['Component', 'Version'])
    csv_out.writerow(['cpu', cpu_name])
    csv_out.writerow(['gpu', gpu_name])

    current_module = sys.modules[__name__]
    for tool, (executable, _) in encoder_data.items():
        if getattr(arguments, tool):
            version_func = getattr(current_module, f'{tool}_version')
            version = version_func()

            csv_out.writerow([tool, version])
    csv_out.writerow([flip_executable, flip_version()])
    csv_out.writerow([magick_executable, magick_version()])


if __name__ == '__main__':
    # Argument parsing and validation.
    args = get_parsed_args()
    args_error = validate_args(args)
    if len(args_error) > 0:
        sys.exit(args_error)

    if args.info:
        generate_info(args)
