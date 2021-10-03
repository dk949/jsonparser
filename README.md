# README

Reimplementing [this](https://youtu.be/N9RUqGYuGfw) json parser in C++ (originally in haskell).

## Requirements
* python3
* conan
* c++20 compiler (tested on g++11)
* cmake


## To build and run
* On Linux/Mac:
```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/bin/json
```
* On windows:
```sh
# same thing but executable has `.exe` on the end
```
