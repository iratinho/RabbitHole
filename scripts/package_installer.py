from logger import Logger
from typing import Callable
import shutil
import os

class PackageInstaller:
    __binaries_folder_name = "bin"
    __includes_folder_name = "includes"
    __libraries_folder_name = "lib"

    __binaries_extensions = [".dll", ".pdb"]
    __includes_extensions = [".hpp", ".h"]
    __libraries_extensions = [".lib"]

    def InstallPackage(package_dir, target_dir, extension_function: Callable[[str, str, str], None] = None):
        if os.path.exists(package_dir):
            if not os.path.exists(target_dir):
                os.makedirs(target_dir)
            for dir, dir_names, file_names in os.walk(package_dir):
                for file in file_names:
                    file_path = os.path.join(dir, file)
                    # Files that belong to binaries folder
                    for ext in PackageInstaller.__binaries_extensions:
                        if file_path.endswith(ext):
                            bin_dir = os.path.join(target_dir, PackageInstaller.__binaries_folder_name)
                            if not os.path.exists(bin_dir):
                                os.makedirs(bin_dir)
                            Logger.LogAction(f"Copy {file} to {bin_dir}")
                            os.chmod(bin_dir, 0o777)
                            shutil.copyfile(file_path, os.path.join(bin_dir, file))
                    # Files that belong to include folder
                    for ext in PackageInstaller.__includes_extensions:
                        if file_path.endswith(ext):
                            include_dir = os.path.join(target_dir, PackageInstaller.__includes_folder_name)
                            if not os.path.exists(include_dir):
                                os.makedirs(include_dir)
                            Logger.LogAction(f"Copy {file} to {include_dir}")
                            os.chmod(include_dir, 0o777)
                            shutil.copyfile(file_path, os.path.join(include_dir, file))
                    # Files that belong to lib
                    for ext in PackageInstaller.__libraries_extensions:
                        if file_path.endswith(ext):
                            lib_dir = os.path.join(target_dir, PackageInstaller.__libraries_folder_name)
                            if not os.path.exists(lib_dir):
                                os.makedirs(lib_dir)
                            Logger.LogAction(f"Copy {file} to {lib_dir}")
                            os.chmod(lib_dir, 0o777)
                            shutil.copyfile(file_path, os.path.join(lib_dir, file))
                    # Allow the oporotunity to inject custom install logic for special cases
                    if extension_function is not None:
                        extension_function(file, dir, target_dir)