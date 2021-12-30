# Voxer

Remote scientific visualization service

> This repository only contains the backend service

![teaser](teaser.png)

## Getting Started

You can

- build and run the server using Docker
- or build voxer server from source manually

### Using the Docker Image

You should use `nvidia-docker 2.0` to enable GPU rendering capability.
For more information and system requirement about `nvidia-docker` please check [nvidia-docker](https://github.com/NVIDIA/nvidia-docker) 
[deployment-guide](https://docs.nvidia.com/ai-enterprise/deployment-guide/dg-docker.html).
```shell
cd voxer
docker build . -t voxer # build the image
```

### Building from Source

#### Prerequisites

1. Any c++ compiler supporting **C++17**
1. CMake v3.11 or newer
1. [OSPRay](http://www.ospray.org) >= v1.8.5. (v2.0.0 not supported)
1. zlib
1. NodeJS & npm

#### Compiling on Linux

```shell
$ cd voxer
$ mkdir build && cd build
$ cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/target/directory \
  -Dospray_DIR=/path/to/ospray-config.cmake \
$ cmake --build . -j 4
$ cmake --build . --target install # install command
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
