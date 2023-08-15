#!/usr/bin/env python3

from io import BytesIO
from os import getcwd, path, rename
from requests import get
from platform import architecture, processor, system
from sys import exit
import tarfile
from zipfile import ZipFile

# Setup environment
ispc_path = getcwd()
is_zip = False

arch = architecture()[0]
processor = processor()
system = system()

print("Getting latest release of ispc...")

raw = get("https://api.github.com/repos/ispc/ispc/releases/latest")
json_response = raw.json()
tag_name = json_response["tag_name"]

print(f"Latest release: {tag_name}\n")
# print(f'{json_response["body"]}\n')

if system == "Darwin":
    if processor == "arm":
        print(f"Darwin/MacOS system detected with a {arch} {processor} CPU...")
        target_archive = f"ispc-{tag_name}-macOS.{processor.lower()}.tar.gz"
    elif processor == "i386":
        print(f"Darwin/MacOS system detected with a {arch} {processor} CPU...")
        target_archive = f"ispc-{tag_name}-macOS.tar.gz"
    else:
        print(f"Unsupported processor {system} {arch} {processor}")
elif system == "Linux":
    print(f"Linux system detected with a {arch} {processor} CPU...")
    target_archive = f"ispc-{tag_name}-{system.lower()}.tar.gz"
elif system == "Windows":
    print(f"Windows system detected with a {arch} {processor} CPU...")
    target_archive = f"ispc-{tag_name}-{system.lower()}.zip"
    is_zip = True
else:
    print(f"Unsupported system {system} {arch} {processor}")
    exit()

# Try to find a valid release
for asset in json_response["assets"]:
    if asset["name"] == target_archive:
        browser_download_url = asset["browser_download_url"]
if not "browser_download_url" in locals():
    print(f"Failed to find valid ispc/ispc release for {system} {arch} {processor}")
    exit()
# Try to download & extract ispc release from browser_download_url
if is_zip:
    target_archive_extracted = target_archive.replace(".zip", "")
    try:
        print(f"Downloading & extracting ispc release from: {browser_download_url}")
        with ZipFile(BytesIO(get(browser_download_url).content)) as zipobj:
            zipobj.extractall(ispc_path)
    except:
        print(f"Failed to download: {browser_download_url}")
        print(
            "Did the file/url change?\nDoes your environment have access to the internet?"
        )
else:
    target_archive_extracted = target_archive.replace(".tar.gz", "")
    try:
        print(f"Downloading and extracting ISPC release from: {browser_download_url}")
        with get(browser_download_url, stream=True) as rx, tarfile.open(
            fileobj=rx.raw, mode="r:gz"
        ) as tarobj:
            tarobj.extractall(ispc_path)
    except:
        print(f"Failed to download: {browser_download_url}")
        print(
            "Did the file/url change?\nDoes your environment have access to the internet?"
        )
# Rename the release to be common name
rename(target_archive_extracted, "ispc")
