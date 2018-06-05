FROM ubuntu:xenial

ARG build_parallel

ADD temp/sources.list /etc/apt/
RUN apt-get update && \
    apt-get install -y \
            build-essential \
            cmake \
            mpich \
            libmpich-dev \
    && \
    rm -rf /var/lib/apt/lists/*

ADD temp/poco-1.9.0.tar.gz /opt/
RUN mv /opt/poco-1.9.0 /opt/poco && \
    cd /opt/poco && \
    ./configure && \
    make ${build_parallel} && \
    make install && \
    rm -rf /opt/poco

ADD temp/ospray-1.6.0.tar.gz /opt/
RUN mv /opt/ospray-1.6.0 /opt/ospray

COPY ParallelRenderer /opt/vovis/ParallelRenderer
COPY third_party /opt/vovis/third_party
COPY CMakeLists.txt /opt/vovis
WORKDIR /opt/vovis/build
RUN cmake .. \
   -DOSPRAY_LIB_PATH=/opt/ospray/lib \
   -DOSPRAY_INCLUDE_PATH=/opt/ospray/include \
   -DEMBREE_LIB_PATH=/opt/ospray/lib \
   -DTBB_LIB_PATH=/opt/ospray/lib \
   -DMPI_INCLUDE_PATH=/usr/include/mpi && \
    make ${build_parallel}

CMD ["echo", "ok"]