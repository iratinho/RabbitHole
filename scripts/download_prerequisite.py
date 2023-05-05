import os
import shutil
import requests
import platform
from logger import Logger
from colorama import init, Fore, Style
from package_installer import PackageInstaller
from package_downloader import PackageDownlaoder

init()

username = "ultralight-ux"
repository = "Ultralight"
tag = "v1.2.1"
download_dir = "./.download_artifacts"
install_dir = "./.build/generated_install/"

ultralight_package = "ultralight-sdk-1.2.1-win-x64.7z" if platform.system() == "Windows" else "ultralight-sdk-1.2.1-linux-x64.7z"
url = f"https://github.com/{username}/{repository}/releases/download/{tag}/{ultralight_package}"

PackageDownlaoder.GitDownloadLibrary(url, tag, ultralight_package, os.path.join(download_dir, "ultralight"))
PackageInstaller.InstallPackage(os.path.join(download_dir, "ultralight"), os.path.join(install_dir, "ultralight"))

# Delete download_artifacts directory
if os.path.exists(download_dir):
    shutil.rmtree(download_dir)