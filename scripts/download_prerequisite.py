import os
import platform
from colorama import init
from package_installer import PackageInstaller
from package_downloader import PackageDownlaoder

def setup_ultralight():
    ultralight_tag = "1.4.0-beta"

    packages = {
        "Windows": {"x86_64": f"ultralight-sdk-{ultralight_tag.replace("-beta", "b")}-win-x64.7z"},
        "Darwin": {"arm64": f"ultralight-sdk-{ultralight_tag.replace("-beta", "b")}-mac-arm64.7z"},
    }

    ultralight_package = packages[platform.system()][platform.machine()]
    url = f"https://github.com/ultralight-ux/Ultralight/releases/download/v{ultralight_tag}/{ultralight_package}"
    download_dir = "./.download_artifacts/ultralight/"
    install_dir = "./.build/generated_install/"

    PackageDownlaoder.GitDownloadLibrary(url, ultralight_package, os.path.join(download_dir, "ultralight"))
    PackageInstaller.InstallPackage(os.path.join(download_dir, "ultralight"), os.path.join(install_dir, "ultralight"))

if __name__ == "__main__":
    init()
    setup_ultralight()




# Download MoltenVK for macos only since linux and windows have native vulkan support
#if(platform.system() == "Darwin"):
#    moltenVK_tag = "v1.2.3"
#    moltenVK_package = "macos.tar"
#    moltenVK_url = f"https://github.com/KhronosGroup/MoltenVK/suites/11749022866/artifacts/612167165"
#    PackageDownlaoder.GitDownloadLibrary(moltenVK_url, moltenVK_tag, moltenVK_package, os.path.join(download_dir, "moltenVK"))
#    PackageInstaller.InstallPackage(os.path.join(download_dir, "moltenVK"), os.path.join(install_dir, "moltenVK"))

# Delete download_artifacts directory
# if os.path.exists(download_dir):
#     shutil.rmtree(download_dir)