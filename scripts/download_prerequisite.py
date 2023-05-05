import requests
import os
import py7zr
from colorama import init, Fore, Style
import time;
import shutil
from logger import Logger
from package_downloader import PackageDownlaoder
from package_installer import PackageInstaller

init()

username = "ultralight-ux"
repository = "Ultralight"
tag = "v1.2.1"
filename = "ultralight-sdk-1.2.1-win-x64.7z"
download_dir = "./.download_artifacts"
install_dir = "./.build/generated_install/"

url = f"https://github.com/{username}/{repository}/releases/download/{tag}/{filename}"

PackageDownlaoder.GitDownloadLibrary(url, tag, filename, os.path.join(download_dir, "ultralight"))
PackageInstaller.InstallPackage(os.path.join(download_dir, "ultralight"), os.path.join(install_dir, "ultralight"))

# Delete download_artifacts directory
if os.path.exists(download_dir):
    shutil.rmtree(download_dir)