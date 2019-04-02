FROM mpi:ubuntu

ARG build_parallel

USER root

RUN sed -i 's/deb.debian.org/mirrors.ustc.edu.cn/g' /etc/apt/sources.list && apt-get update
RUN apt-get install -y \
            libgl1-mesa-dev \
            libglew-dev \
            libxt-dev
    # && \
    # rm -rf /var/lib/apt/lists/*

ADD docker/deps/ospray-1.8.2.x86_64.linux.tar.gz /tmp/
RUN mv /tmp/ospray-1.8.2.x86_64.linux/lib/* /usr/lib && \
    mv /tmp/ospray-1.8.2.x86_64.linux/include/* /usr/include && \
    rm -rf /tmp/ospray-1.8.2.x86_64.linux

ADD docker/deps/poco-1.9.0.tar.gz /tmp/
RUN mv /tmp/poco-1.9.0 /tmp/poco && \
    mkdir /tmp/poco/cmake-build && \
    cd /tmp/poco/cmake-build && \
    cmake ..  -DCMAKE_INSTALL_PREFIX=/usr/ && \
    cmake --build . -- -j4 && \
    cmake --install . && \
    make install && \
    rm -rf /tmp/poco

ADD docker/deps/vtk-v8.2.0.tar.gz /tmp/
RUN mv /tmp/vtk-v8.2.0 /tmp/vtk && \
    mkdir /tmp/vtk/build && \
    cd /tmp/vtk/build && \
    cmake .. -DBUILD_TESTING=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/ && \
    cmake --build . -- -j4 && \
    cmake --install . && \
    make install && \
    rm -rf /tmp/vtk

COPY ParallelRenderer /tmp/vovis/ParallelRenderer
COPY app /tmp/vovis/app
COPY test /tmp/vovis/test
COPY third_party /tmp/vovis/third_party
COPY CMakeLists.txt /tmp/vovis
WORKDIR /tmp/vovis/build
RUN cmake .. && cmake --build . -- -j4

USER mpi

CMD ["/bin/bash"]