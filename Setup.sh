#!/usr/bin/env zsh

python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install -r requirements.txt

#python3 scripts/download_prerequisite.py

# Generate main project
cmake -S . -B .build -G Xcode -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=.build/install -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
