# Prerequisites
To build and run the renderer in the server, you need to have installed
1. [Ospray](http://www.ospray.org) v1.6.1 or newer.
2. MPI library, etc MPICH2, OpenMPI, Intel MPI.
3. [Poco](https://pocoproject.org/) Library v1.8.1 or newer.
4. CMake, any C++ compiler supporting C++11 and standard Linux development tools.

To build and bundle the frontend files, you need
1. [NodeJS](https://nodejs.org/) v8.11.3 or newer.
2. [Yarn](https://yarnpkg.com) v1.7.0 or newer.

# Build
Build server renderer
``` shell
cd ParallelRenderer
mkdir build
cd build
cmake ..
# or cmake .. \
# -DOSPRAY_LIB_PATH=/your_path \
# -DOSPRAY_INCLUDE_DIR=/your_path \
# -DMPI_COMPILER_PATH=/your_path \
# -DPOCO_LIB_PATH=/your_path \
# -DPOCO_INCLUDE_DIR=/your_path
make
```

Build webpage
``` shell
cd web
npm install
npm run build
```