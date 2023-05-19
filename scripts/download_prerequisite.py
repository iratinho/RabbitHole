import os
import platform
import shutil
from logger import Logger
from colorama import init, Fore, Style
from package_installer import PackageInstaller
from package_downloader import PackageDownlaoder

init()

download_dir = "./.download_artifacts"
install_dir = "./.build/generated_install/"

# Download ultralight 
ultralight_tag = "v1.2.1"
ultralight_package = ""
if(platform.system() == "Windows"):
    ultralight_package = "ultralight-sdk-1.2.1-win-x64.7z"
if(platform.system() == "linux" or platform.system() == "linux2"):
    ultralight_package = "ultralight-sdk-1.2.1-linux-x64.7z"
if(platform.system() == "Darwin"):
    ultralight_package = "ultralight-sdk-1.2.1-mac-x64.7z"
url = f"https://github.com/ultralight-ux/Ultralight/releases/download/{ultralight_tag}/{ultralight_package}"
PackageDownlaoder.GitDownloadLibrary(url, ultralight_tag, ultralight_package, os.path.join(download_dir, "ultralight"))
PackageInstaller.InstallPackage(os.path.join(download_dir, "ultralight"), os.path.join(install_dir, "ultralight"))

# Download MoltenVK for macos only since linux and windows have native vulkan support
if(platform.system() == "Darwin"):
    moltenVK_tag = "test-release-CI-4"
    moltenVK_package = "MoltenVK-macos.tar"
    moltenVK_url = f"https://github.com/KhronosGroup/MoltenVK/releases/download/{moltenVK_tag}/{moltenVK_package}"
    PackageDownlaoder.GitDownloadLibrary(moltenVK_url, moltenVK_tag, moltenVK_package, os.path.join(download_dir, "moltenVK"))
    PackageInstaller.InstallPackage(os.path.join(download_dir, "moltenVK"), os.path.join(install_dir, "moltenVK"))

# Delete download_artifacts directory
if os.path.exists(download_dir):
    shutil.rmtree(download_dir)