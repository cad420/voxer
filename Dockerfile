FROM ubuntu:xenial

ARG build_parallel

RUN sed -i 's/http:\/\/archive\.ubuntu\.com\/ubuntu\//http:\/\/mirrors\.163\.com\/ubuntu\//g' /etc/apt/sources.list && \
    apt-get update && \
    apt-get install -y \
            build-essential \
            cmake \
            mpich \
            libmpich-dev \
    && \
    rm -rf /var/lib/apt/lists/*

ADD temp/ispc-v1.9.2-linux.tar.gz /opt/
RUN mv /opt/ispc-v1.9.2-linux /opt/ispc && \
    update-alternatives --install /usr/bin/ispc ispc /opt/ispc/ispc 1

ADD temp/tbb2018_20180312oss_lin.tar.gz /opt/
RUN mv /opt/tbb2018_20180312oss /opt/tbb

ADD temp/embree-3.2.0.x86_64.linux.tar.gz /opt/
RUN mv /opt/embree-3.2.0.x86_64.linux /opt/embree

ADD temp/ospray-1.6.0.tar.gz /opt/
RUN mv /opt/ospray-1.6.0 /opt/ospray && \
    mkdir /opt/ospray/build && \
    cd /opt/ospray/build && \
    cmake .. \
          -Dembree_DIR=/opt/embree \
          -DOSPRAY_ENABLE_APPS:BOOL=OFF \
          -DOSPRAY_APPS_BENCHMARK:BOOL=OFF \
          -DOSPRAY_MODULE_MPI:BOOL=ON \
          -DOSPRAY_APPS_EXAMPLEVIEWER:BOOL=OFF \
          -DOSPRAY_APPS_BENCHMARK:BOOL=OFF \
          -DTBB_ROOT=/opt/tbb \
    && \
    make ${build_parallel} && \
    make install && \
    rm -rf /opt/ospray

ADD temp/poco-1.9.0.tar.gz /opt/
RUN mv /opt/poco-1.9.0 /opt/poco && \
    cd /opt/poco && \
    ./configure && \
    make ${build_parallel} && \
    make install && \
    rm -rf /opt/poco

COPY ParallelRenderer /opt/vovis/ParallelRenderer
COPY third_party /opt/vovis/third_party
COPY CMakeLists.txt /opt/vovis
WORKDIR /opt/vovis/build
RUN cmake .. \
   -Dembree_DIR=/opt/embree \
   -DTBB_ROOT=/opt/tbb && \
    make ${build_parallel}

CMD ["sh", "-c", "ls"]