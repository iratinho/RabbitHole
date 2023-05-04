import os
import sys
import shutil
import time
from colorama import init, Fore, Style

print()
print(Fore.GREEN + "Running Utility script to copy webpack artifacts.")

start_time = time.time()

# This is the directory to where the webpack artifacts will be copy into
target_dir = os.path.abspath("../.build/src/app/ultralight/assets")

if not os.path.exists(target_dir):
    print(Fore.BLUE + "[Info]: " + Style.RESET_ALL + f"Creating directory to copy webpack artifacts into {target_dir}")
    os.makedirs(target_dir)

# This is the directory where webpack creates the bundled files
webpack_dir = os.path.abspath("../src/html/app/dist/")

if not os.path.exists(webpack_dir):
    print(Fore.RED + "[Error]: " + Style.RESET_ALL + f"Aborting, there are no webpack artifacts to copy.")
    sys.exit(1)


print(Fore.BLUE + "[Info]: " + Style.RESET_ALL + f"Copying all artifacts from {webpack_dir} into {target_dir}")
shutil.copytree(webpack_dir, target_dir, dirs_exist_ok=True)

end_timer = time.time()

print(Fore.GREEN + f"Finished in {(end_timer - start_time):.2f} seconds.")
