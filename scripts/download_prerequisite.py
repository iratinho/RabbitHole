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

# Custom function to copy the ultralight resources that are not handle by the PackageInstaller
def CopyUltralightResources(file: str, dir: str, target_dir: str):
    if file.endswith(".pem") or file.endswith(".dat"):
        resources_dir = os.path.join(target_dir, "bin", "resources")
        if not os.path.exists(resources_dir):
            os.makedirs(resources_dir)
        Logger.LogAction(f"Copy {file} to {resources_dir}")
        shutil.copyfile(os.path.join(dir, file), os.path.join(target_dir, "bin", "resources", file))
        
PackageDownlaoder.GitDownloadLibrary(url, tag, filename, os.path.join(download_dir, "ultralight"))
PackageInstaller.InstallPackage(os.path.join(download_dir, "ultralight"), os.path.join(install_dir, "ultralight"), CopyUltralightResources)

# Delete download_artifacts directory
if os.path.exists(download_dir):
    shutil.rmtree(download_dir)