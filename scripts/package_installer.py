from logger import Logger
from typing import Callable
import shutil

class PackageInstaller:
    def InstallPackage(package_dir: str, target_dir):
        Logger.LogAction(f"Installing package into {target_dir}")
        shutil.copytree(package_dir, target_dir, dirs_exist_ok=True)
