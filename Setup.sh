python -m venv venv
. ./venv/bin/activate
python -m pip install -U logger
python -m pip install -U colorama
. ./venv/bin/activate

# Generate and build ExternalLibraries project
cmake -S ./external -B ./external/.build -G Xcode -DCMAKE_BUILD_TYPE=Debug -DASSIMP_SOURCE=ON -DFMT_SOURCE=ON -DGLFW_SOURCE=ON -DCMAKE_INSTALL_PREFIX=.build/generated_install/
cmake --build ./external/.build --config Debug

# Generate main project
cmake -S . -B .build -G Xcode -DCMAKE_BUILD_TYPE=Debug