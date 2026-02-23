# Release build
mkdir -p build/Release
cd build/Release
cmake -DCMAKE_BUILD_TYPE=Release ../..
cmake --build .

# Debug build
mkdir -p build/Debug
cd build/Debug
cmake -DCMAKE_BUILD_TYPE=Debug ../..
cmake --build .

# Run executables
- build/Release/mittelvec
- build/Debug/mittelvec

# Single-Header Library
1. Generate the header: `python3 generate_single_header.py`
2. Compile the test app: `clang++ -std=c++17 header_test.cpp -o header_test -lpthread`
3. Run: `./header_test`