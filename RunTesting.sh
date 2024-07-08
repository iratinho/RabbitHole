cmake --build .build --config Debug
cd .build && ctest -C Debug
cd -