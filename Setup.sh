python -m venv venv
. ./venv/bin/activate
python -m pip install -U logger
python -m pip install -U colorama
. ./venv/bin/activate

# Generate main project
cmake -DCMAKE_VERBOSE_MAKEFILE=ON -S . -B .build -G Xcode -DCMAKE_BUILD_TYPE=Debug