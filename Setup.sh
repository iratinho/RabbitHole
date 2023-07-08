python -m venv venv
. ./venv/bin/activate
python -m pip install -U logger
python -m pip install -U colorama
. ./venv/bin/activate
cmake -S . -B .build -G Xcode