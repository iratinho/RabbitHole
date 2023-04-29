import requests
import os
import py7zr
from colorama import init, Fore, Style
import time;
import shutil

init()

username = "ultralight-ux"
repository = "Ultralight"
tag = "v1.2.1"
filename = "ultralight-sdk-1.2.1-win-x64.7z"
output_dir = "./.download_artifacts"
install_dir = "./.build/generated_install/"

url = f"https://github.com/{username}/{repository}/releases/download/{tag}/{filename}"

response = requests.get(url)

archive_path = os.path.join(os.path.abspath(output_dir), filename)
abs_install_dir = os.path.join(os.path.abspath(install_dir), "ultralight")

start_time = time.time()

if not os.path.exists(os.path.abspath(output_dir)):
    os.makedirs(output_dir)

print(Fore.GREEN + "Running Utility script to download ultralight SDK.")

print(Fore.BLUE + "[Info]: " + Style.RESET_ALL + f"Downloading {url}")

with open(archive_path, "wb") as f:
    f.write(response.content)

if os.path.splitext(archive_path)[1] == ".7z":
    print(Fore.BLUE + "[Info]: " + Style.RESET_ALL + f"Extracting 7z file. {url}")
    extract_dir = os.path.splitext(archive_path)[0]
    # Extract the zip file using 7zip
    with py7zr.SevenZipFile(archive_path, 'r') as archive:
        archive.extractall(extract_dir)

print(Fore.BLUE + "[Info]: " + Style.RESET_ALL + f"Removing archive.")
os.remove(archive_path)

print(Fore.BLUE + "[Info]: " + Style.RESET_ALL + f"Installing in cmake directory.")

if not os.path.exists(os.path.abspath(abs_install_dir)):
    print(Fore.BLUE + "[Info]: " + Style.RESET_ALL + f"Creating directory: {abs_install_dir}")
    os.makedirs(abs_install_dir)

ultralight_include_path = os.path.join(extract_dir, "include")
if not os.path.exists(ultralight_include_path):
    print(Fore.RED + "[Error]: " + Style.RESET_ALL + f"Unable to find include path.")
else:
    target_path = os.path.join(abs_install_dir, "include")
    if not os.path.exists(target_path):
        print(Fore.BLUE + "[Info]: " + Style.RESET_ALL + f"Moving include contents.")
        # Copy a directory and all its contents to a new location
        shutil.copytree(ultralight_include_path, target_path)

ultralight_lib_path = os.path.join(extract_dir, "lib")
if not os.path.exists(ultralight_lib_path):
    print(Fore.RED + "[Error]: " + Style.RESET_ALL + f"Unable to find lib path.")
else:
    target_path = os.path.join(abs_install_dir, "lib")
    if not os.path.exists(target_path):
        print(Fore.BLUE + "[Info]: " + Style.RESET_ALL + f"Moving lib contents.")
        # Copy a directory and all its contents to a new location
        shutil.copytree(ultralight_lib_path, target_path)

ultralight_dll_path = os.path.join(extract_dir, "bin")
if not os.path.exists(ultralight_dll_path):
    print(Fore.RED + "[Error]: " + Style.RESET_ALL + f"Unable to find binaries path.")
else:
    target_path = os.path.join(abs_install_dir, "bin")
    if not os.path.exists(target_path):
        print(Fore.BLUE + "[Info]: " + Style.RESET_ALL + f"Moving binaries contents.")
        # Copy a directory and all its contents to a new location
        shutil.copytree(ultralight_dll_path, target_path)


end_timer = time.time()

print(Fore.GREEN + f"Finished in {(end_timer - start_time):.2f} seconds.")
