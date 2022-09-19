import os
import subprocess

def glfw3_setup():
    os.chdir(os.getcwd() + "/external/glfw")

    # Set the install directory for current cmake proj
    subprocess.check_call('cmake ' + '-DCMAKE_INSTALL_PREFIX:PATH=' + os.getcwd() + ' ' + '-B .build -S .')

    # Install cmake so it generate config to be used with find_packge
    subprocess.check_call('cmake --build .build --target install')

def main():
    glfw3_setup()

if __name__ == '__main__':
    main()