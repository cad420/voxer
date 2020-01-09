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

RUN git clone --depth=1 https://github.com/uNetworking/uSockets.git \
  && cd uSockets \
  && make \
  && cp uSockets.a libuSockets.a

RUN git clone --depth=1 https://github.com/uNetworking/uWebSockets.git \
  && cd uWebSockets \
  && cp -r src uwebsockets

RUN git clone --depth=1 https://github.com/Tencent/rapidjson.git

RUN git clone --depth=1 https://github.com/fmtlib/fmt.git && mkdir -p fmt/build && cd fmt/build \
  && cmake .. -DFMT_TEST=OFF -DFMT_DOC=OFF -DCMAKE_INSTALL_PREFIX=./fmt && \
  cmake --build . --target install

RUN ls /tmp/ospray-1.8.5.x86_64.linux
ADD . /tmp/voxer
WORKDIR /tmp/voxer
RUN mkdir build && cd build && \
  cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -Dospray_DIR=/tmp/ospray-1.8.5.x86_64.linux/lib/cmake/ospray-1.8.5 \
  -DRAPIDJSON_INCLUDE_PATH=/tmp/rapidjson/include \
  -DUSOCKETS_LIB_PATH=/tmp/uSockets/ \
  -DUSOCKETS_INCLUDE_PATH=/tmp/uSockets/src \
  -DUWEBSOCKETS_INCLUDE_PATH=/tmp/uWebSockets/ \
  -Dfmt_DIR=/tmp/fmt/build/fmt/lib/cmake/fmt \
  -DCMAKE_INSTALL_PREFIX=/tmp/voxer-install && \
  cmake --build . --target install

RUN cp -r /tmp/ospray-1.8.5.x86_64.linux/lib/* /tmp/voxer-install/lib
RUN cp /usr/lib/x86_64-linux-gnu/libz.so* /tmp/voxer-install/lib/

FROM ubuntu:19.10
COPY --from=builder /tmp/voxer-install /tmp/voxer

EXPOSE 3000

CMD ["/tmp/voxer/bin/voxer-server", "/tmp/data/datasets.json", "/tmp/data"]