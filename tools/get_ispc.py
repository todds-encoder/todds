#!/usr/bin/env python3

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

from io import BytesIO
from os import getcwd, path, rename
from requests import get
from platform import architecture, processor, system
from shutil import rmtree
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

if system == "Darwin":
    print(f"Darwin/MacOS system detected...")
    target_archive = f"ispc-{tag_name}-macOS.universal.tar.gz"
elif system == "Linux":
    print(f"Linux system detected with a {arch} {processor} CPU...")
    target_archive = f"ispc-{tag_name}-{system.lower()}-oneapi.tar.gz"
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
# Reinstall latest ispc relative to project root. Rename the release to be common name.
target_archive_extracted = target_archive_extracted.replace("-oneapi", "")
if path.exists("ispc"):
    rmtree("ispc")
rename(target_archive_extracted, "ispc")
