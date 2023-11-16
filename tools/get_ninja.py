#!/usr/bin/env python3

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

from io import BytesIO
from os import chdir, getcwd, mkdir, path, rename
from requests import get
from platform import architecture, processor, system
from shutil import rmtree
from sys import exit
import tarfile
from zipfile import ZipFile

arch = architecture()[0]
processor = processor()
system = system()

if system == "Darwin":
    print(f"Please install via brew/MacPorts...")
    exit()
elif system == "Linux":
    print(f"Please install ninja via your preferred package manager...")
    exit()
elif system == "Windows":
    print(f"Windows system detected with a {arch} {processor} CPU...")
    target_archive = f"ninja-win.zip"
else:
    print(f"Unsupported system {system} {arch} {processor}")
    exit()

print("Getting latest release of ninja...")

raw = get("https://api.github.com/repos/ninja-build/ninja/releases/latest")
json_response = raw.json()
tag_name = json_response["tag_name"]

print(f"Latest release: {tag_name}\n")

# Try to find a valid release
for asset in json_response["assets"]:
    if asset["name"] == target_archive:
        browser_download_url = asset["browser_download_url"]
if not "browser_download_url" in locals():
    print(
        f"Failed to find valid ninja-build/ninja release for {system} {arch} {processor}"
    )
    exit()
# Reinstall latest ispc relative to project root. Rename the release to be common name.
if path.exists("ninja"):
    rmtree("ninja")
mkdir("ninja")
try:
    print(f"Downloading & extracting ispc release from: {browser_download_url}")
    with ZipFile(BytesIO(get(browser_download_url).content)) as zipobj:
        zipobj.extractall("ninja")
except:
    print(f"Failed to download: {browser_download_url}")
    print(
        "Did the file/url change?\nDoes your environment have access to the internet?"
    )
