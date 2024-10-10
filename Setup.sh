#!/usr/bin/env zsh

python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install -r requirements.txt

#python3 scripts/download_prerequisite.py
# https://stunlock.gg/posts/emscripten_with_cmake/
# https://github.com/eliemichel/glfw3webgpu/blob/main/CMakeLists.txt
# https://eliemichel.github.io/LearnWebGPU/getting-started/project-setup.html
emcmake cmake -S . -B .build -DASSIMP_BUILD_ZLIB=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=.build/install -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DAPPLE=1 -DUSE_VULKAN=FALSE

# Generate main project
#cmake -S . -B .build -G Xcode -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=.build/install -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

