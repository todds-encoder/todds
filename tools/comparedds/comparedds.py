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
import pathlib
import shutil
import subprocess
import sys
import time
import winreg

# Tools used by this script.
bc7enc_tool = 'bc7enc'
nvtt_tool = 'nvtt'
png2dds_tool = 'png2dds'
texconv_tool = 'texconv'
nvdecompress_executable = 'nvdecompress'
flip_executable = 'flip'
magick_executable = 'magick'

# Maps tool arguments to their executable and parameters.
EncoderData = collections.namedtuple('EncoderData', 'executable batch filepath params')
encoder_data = {
    bc7enc_tool: EncoderData('bc7enc', False, True, ('-q', '-g', '-u6')),
    nvtt_tool: EncoderData('nvbatchcompress', True, True, ('-fast', '-bc7', '-silent')),
    png2dds_tool: EncoderData('png2dds', True, False, ('-o',)),
    texconv_tool: EncoderData('texconv', True, False, ('-y', '-f', 'BC7_UNORM', '-bc', 'x')),
}


def get_parsed_args():
    parser = argparse.ArgumentParser(description='Compare DDS encoders.')
    for tool, data in encoder_data.items():
        parser.add_argument(f'--{tool}', action='store_true',
                            help=f'Compare {tool}. The {data.executable} executable must be in the PATH.')
    parser.add_argument(f'--info', action='store_true', help=f'Generate a CSV with system and tool information')
    parser.add_argument(f'--batch', action='store_true',
                        help=f'Generate a CSV with the time required to process all inputs in batch mode.')
    parser.add_argument(f'--files', action='store_true',
                        help=f'Generate a CSV with the encoding time for individual files along with file statistics.')
    parser.add_argument(f'--metrics', action='store_true',
                        help=f'Generate a CSV with metrics for DDS files. Will fail if they have not been generated')
    parser.add_argument('input', metavar='input', type=str, help='Path containing PNGs to be used for testing')
    parser.add_argument('output', metavar='output', type=str, help='Path in which output PNGs will be created.')
    return parser.parse_args()


def validate_args(arguments):
    for tool, data in encoder_data.items():
        if getattr(arguments, tool) and shutil.which(data.executable) is None:
            return f'To use {tool}, {data.executable} must be present in the PATH'

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

    csv_out = csv.writer(sys.stdout, lineterminator='\n')
    csv_out.writerow(['Component', 'Version'])
    csv_out.writerow(['cpu', cpu_name])
    csv_out.writerow(['gpu', gpu_name])

    current_module = sys.modules[__name__]
    for tool in encoder_data.keys():
        if getattr(arguments, tool):
            version_func = getattr(current_module, f'{tool}_version')
            version = version_func()

            csv_out.writerow([tool, version])
    csv_out.writerow([flip_executable, flip_version()])
    csv_out.writerow([magick_executable, magick_version()])


def output_file_path(input_file, output_path):
    return os.path.join(output_path, pathlib.Path(input_file).stem)


def bc7enc_execute(input_file, output_file):
    bc7enc_data = encoder_data[bc7enc_tool]
    arguments = [bc7enc_data.executable, ]
    arguments.extend(bc7enc_data.params)
    arguments.append(input_file)
    arguments.append(output_file)
    return subprocess.Popen(arguments, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def nvtt_execute(input_path, output_path):
    nvtt_data = encoder_data[nvtt_tool]
    arguments = [nvtt_data.executable, ]
    arguments.extend(nvtt_data.params)
    arguments.append(input_path)
    arguments.append(output_path)
    return subprocess.Popen(arguments, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def png2dds_execute(input_path, output_path):
    png2dds_data = encoder_data[png2dds_tool]
    arguments = [png2dds_data.executable, ]
    arguments.extend(png2dds_data.params)
    arguments.append(input_path)
    arguments.append(output_path)
    return subprocess.Popen(arguments, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def texconv_execute(input_path, output_path):
    texconv_data = encoder_data[texconv_tool]
    arguments = [texconv_data.executable, ]
    arguments.extend(texconv_data.params)
    input_string = input_path
    if os.path.isdir(input_path):
        arguments.append('-r')
        input_string = os.path.join(input_path, '*.png')
    arguments.append(input_string)
    arguments.append('-o')
    arguments.append(output_path)
    return subprocess.Popen(arguments, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def batch_encode(arguments, input_files):
    current_module = sys.modules[__name__]
    csv_out = csv.writer(sys.stdout, lineterminator='\n')
    csv_out.writerow(['Batch conversion', 'Time (ns)'])

    for tool, data in encoder_data.items():
        if getattr(arguments, tool):
            output_path = os.path.join(args.output, tool)
            pathlib.Path(output_path).mkdir(parents=True, exist_ok=True)

            execute_func = getattr(current_module, f'{tool}_execute')
            batch_start = time.perf_counter_ns()
            if data.batch:
                execute_func(arguments.input, output_path).wait()
            else:
                processes = [execute_func(input_file, output_file_path(input_file, output_path) + '.dds') for input_file
                             in input_files]
                for process in processes:
                    process.wait()

            batch_time = time.perf_counter_ns() - batch_start
            csv_out.writerow([tool, batch_time])


def files_encode(arguments, input_files):
    current_module = sys.modules[__name__]
    csv_out = csv.writer(sys.stdout, lineterminator='\n')
    csv_out.writerow(['File', 'Tool', 'Time (ns)', 'Size (Bytes)'])
    for tool, data in encoder_data.items():
        if getattr(arguments, tool):
            output_path = os.path.join(args.output, tool)
            pathlib.Path(output_path).mkdir(parents=True, exist_ok=True)

            execute_func = getattr(current_module, f'{tool}_execute')
            for input_file in input_files:
                output_file = output_file_path(input_file, output_path) + '.dds'
                execute_start = time.perf_counter_ns()
                execute_func(input_file, output_file if data.filepath else output_path).wait()
                execute_time = time.perf_counter_ns() - execute_start
                csv_out.writerow([input_file, tool, execute_time, os.path.getsize(output_file)])


def decode_png(dds_file, png_file):
    subprocess.run([nvdecompress_executable, '-format', 'png', dds_file, png_file], stdout=subprocess.DEVNULL)


def calculate_metric(png_input, png_file, metric):
    arguments = [magick_executable, 'compare', '-metric', metric, png_input, png_file, 'NULL:']
    output = subprocess.run(arguments, check=False, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE).stderr.decode(
        'utf-8')
    if metric == 'RMSE':
        _, output = output.split(' (')
        output, _ = output.split(')')
    return output


def calculate_flip(png_input, png_file):
    arguments = [flip_executable, '-nexm', '-nerm', '-r', png_input, '-t', png_file]
    lines = subprocess.check_output(arguments, stderr=subprocess.DEVNULL).decode('utf-8').split('\n')
    _, mean = lines[4].rsplit(' ', 1)
    return mean.strip()


def calculate_metrics(arguments, input_files):
    csv_out = csv.writer(sys.stdout, lineterminator='\n')
    csv_out.writerow(['File', 'Tool', 'FLIP (Mean)', 'PSNR', 'RMSE (%)', 'SSIM'])
    for tool, data in encoder_data.items():
        if getattr(arguments, tool):
            output_path = os.path.join(args.output, tool)
            for input_file in input_files:
                output_base = output_file_path(input_file, output_path)
                dds_file = output_base + '.dds'
                png_file = output_base + '.png'
                decode_png(dds_file, png_file)
                flip_mean = calculate_flip(input_file, png_file)
                psnr = calculate_metric(input_file, png_file, 'PSNR')
                rmse = calculate_metric(input_file, png_file, 'RMSE')
                ssim = calculate_metric(input_file, png_file, 'SSIM')
                csv_out.writerow([input_file, tool, flip_mean, psnr, rmse, ssim])


if __name__ == '__main__':
    # Argument parsing and validation.
    args = get_parsed_args()
    args_error = validate_args(args)
    if len(args_error) > 0:
        sys.exit(args_error)

    if args.info or args.metrics:
        for extra_executable in [nvdecompress_executable, magick_executable, flip_executable]:
            if shutil.which(extra_executable) is None:
                sys.exit(f'{extra_executable} must be present in the PATH.')

    if args.info:
        generate_info(args)

    input_file_list = []
    for file in os.listdir(args.input):
        if file.lower().endswith('.png'):
            input_file_list.append(os.path.join(args.input, file))

    if args.batch:
        batch_encode(args, input_file_list)

    if args.files:
        files_encode(args, input_file_list)

    if args.metrics:
        calculate_metrics(args, input_file_list)
