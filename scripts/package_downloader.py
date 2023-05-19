from logger import Logger
import requests
import py7zr
import zipfile
import tarfile
import os

class PackageDownlaoder:
    __archive_extenssions = [".7z", ".zip", ".tar"]

    def GitDownloadLibrary(repository_url: str, tag: str, package_name: str, target_dir: str):
        Logger.LogAction(f"Downloading {package_name} from {repository_url}")

        if not os.path.exists(target_dir):
             os.makedirs(target_dir)

        # Download and write archive to disk
        archive_path = os.path.join(target_dir, package_name)
        PackageDownlaoder.__DownloadArchive(repository_url, archive_path)

        # Extract the package archive if necessary
        for ext in PackageDownlaoder.__archive_extenssions:
            if package_name.endswith(ext):
                Logger.LogAction(f"Extracting {package_name} to {archive_path}")
                PackageDownlaoder.__ExtractPackage(archive_path, target_dir, ext)

    def __DownloadArchive(archive_url: str, output_path: str):
        request = requests.get(archive_url)
        with open(output_path, "wb") as file:
            file.write(request.content)
        
    def __ExtractPackage(package_path: str, output_dir: str, ext: str):
            if(ext == '.7z'):
                with py7zr.SevenZipFile(package_path, "r") as archive:
                    archive.extractall(output_dir)
            if(ext == '.zip'):
                with zipfile.ZipFile(package_path, "r") as archive:
                    archive.extractall(output_dir)
            if(ext == ".tar"):
                with tarfile.TarFile(package_path, "r") as archive:
                    archive.extractall(output_dir)

