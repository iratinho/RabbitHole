#!/usr/bin/env zsh

python3 -m pip install -r requirements.txt

#python3 scripts/download_prerequisite.py
# https://stunlock.gg/posts/emscripten_with_cmake/
# https://github.com/eliemichel/glfw3webgpu/blob/main/CMakeLists.txt
# https://eliemichel.github.io/LearnWebGPU/getting-started/project-setup.html
#emcmake cmake -S . -B .build -G Xcode -DASSIMP_BUILD_ZLIB=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=.build/install -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DAPPLE=1

# Debug - Emscripten
cd ./emsdk && ./emsdk activate latest && source ./emsdk_env.sh && cd -
#emcmake cmake -S . -B .build -DAPPLE=1 -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DASSIMP_BUILD_ZLIB=ON -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=${EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake -DCMAKE_INSTALL_PREFIX=../.build/generated_install/

# Debug - WebGPU Native
cmake -S . -G Xcode -B .build -DWEBGPU_NATIVE=TRUE -DCMAKE_SYSTEM_PROCESSOR=arm64 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_INSTALL_PREFIX=../.build/generated_install/ --graphviz=graph.dot -DAPPLE=1

# Generate main project
#cmake -S . -B .build -G Xcode -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=.build/install -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DAPPLE=1 -DUSE_VULKAN=1

