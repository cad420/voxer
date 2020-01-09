# Voxer

Remote scientific visualization service

> This repository only contains the backend service

## Getting Started

You can
- build and run the server using Docker 
- or build voxer server from source manually

### Using the Docker Image

``` shell
$ cd voxer
$ docker build . -t voxer # build the image
$ mkdir data
$ docker run --rm -p 3000:3000 --mount type=bind,source="$(pwd)/data",target=/tmp/data voxer
```

### Building from Source

#### Prerequisites
1. Any c++ compiler supporting **C++17**
1. CMake v3.10 or newer
1. [OSPRay](http://www.ospray.org) v1.8.5 or newer
1. optional: [vcpkg](https://github.com/microsoft/vcpkg) to install the following packages
1. [RapidJSON](https://github.com/Tencent/rapidjson)
1. [uSockets](https://github.com/uNetworking/uSockets)
1. [uWebSockets](https://github.com/uNetworking/uWebSockets)
1. zlib
1. [fmt](https://github.com/fmtlib/fmt)

You can install the dependencies except OSPRay with `vcpkg` using the following command:
```
$ vcpkg install usockets uwebsockets zlib fmt rapidjson
``` 

If you prefer to install dependencies manually, refer to the `Dockerfile` in the project,
which contains the commands to install dependencies and build the project.

#### Compiling on Linux

With the enviroment variable `VCPKG_ROOT` set, you can run

``` shell
$ cd voxer
$ mkdir build && cd build
$ cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/target/directory \
  -Dospray_DIR=/path/to/ospray-config.cmake \
$ cmake --build . -j 4
$ # cmake --install . # install the building result (require CMake >= 3.15)
$ # cmake --build . --target install # install command before CMake 3.15
```

to build `voxer-server`.

#### Run the Service

TODO

## Citing
If you find `voxer` useful in your research, you can consider citing:
```
@article{yang2019voxer,
  title={Voxer—a platform for creating, customizing, and sharing scientific visualizations},
  author={Yang, Weimin and Tao, Yubo and Lin, Hai},
  journal={Journal of Visualization},
  volume={22},
  number={6},
  pages={1161--1176},
  year={2019},
  publisher={Springer}
}
```