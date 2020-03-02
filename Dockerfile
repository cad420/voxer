FROM ubuntu:19.10 as builder

RUN apt-get update
RUN apt-get install -y \
  build-essential \
  git \
  cmake \
  zlib1g-dev \
  wget

WORKDIR /tmp

RUN wget https://github.com/ospray/OSPRay/releases/download/v1.8.5/ospray-1.8.5.x86_64.linux.tar.gz \
 && tar -xzf ospray-1.8.5.x86_64.linux.tar.gz

ADD . /tmp/voxer
WORKDIR /tmp/voxer
RUN git submodule update --init --depth 1
RUN mkdir build && cd build && \
  cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -Dospray_DIR=/tmp/ospray-1.8.5.x86_64.linux/lib/cmake/ospray-1.8.5 \
  -DCMAKE_INSTALL_PREFIX=/opt/voxer && \
  cmake --build . --target install

RUN cp -r /tmp/ospray-1.8.5.x86_64.linux/lib/* /opt/voxer/lib
RUN cp /usr/lib/x86_64-linux-gnu/libz.so* /opt/voxer/lib/

FROM ubuntu:19.10
COPY --from=builder /opt/voxer /opt/voxer

EXPOSE 3000

CMD ["/opt/voxer/bin/voxer-server"]