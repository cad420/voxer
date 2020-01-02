# Voxer

## Pre
1. Any c++ compiler supporting **C++17**
1. CMake v3.10 or newer
1. [OSPRay](http://www.ospray.org) v1.8.5 or newer
1. [uSockets]()
1. [uWebSockets]()
1. zlib
1. [fmt]()

### Use vcpkg to install dependecies

### Or install dependecies manully

## Build
Build server renderer
``` shell
cd voxer
mkdir build
cd build
cmake ..
# -DOSPRAY_LIB_PATH=/your_path \
# -DOSPRAY_INCLUDE_DIR=/your_path \
# -DMPI_COMPILER_PATH=/your_path \
# -DPOCO_LIB_PATH=/your_path \
# -DPOCO_INCLUDE_DIR=/your_path
make
```

## Run

## Use Docker Image

