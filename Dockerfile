# use ubuntu:20.04 if GPU is not used
FROM nvidia/opengl:1.2-glvnd-devel-ubuntu20.04 as builder

ARG DEBIAN_FRONTEND=noninteractive
ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && apt upgrade -y
RUN apt-get install -y \
      build-essential \
      cmake \
      git \
      zlib1g-dev

COPY . /tmp/voxer
ADD ./third_party/ospray-2.2.0.x86_64.linux.tar.gz /tmp/voxer/third_party
WORKDIR /tmp/voxer/build
RUN cmake .. \
      -DCMAKE_INSTALL_PREFIX=/opt/voxer/ \
      -DCMAKE_BUILD_TYPE=Release \
      # change to OFF if GPU is not used
      -DVOXER_BUILD_BACKEND_GL=ON \
      -Dospray_DIR=/tmp/voxer/third_party/ospray-2.2.0.x86_64.linux/lib/cmake/ospray-2.2.0
RUN cmake --build . && cmake --install .
RUN cp /usr/lib/x86_64-linux-gnu/libz.so* /opt/voxer/lib/
RUN cp -r /tmp/voxer/third_party/ospray-2.2.0.x86_64.linux/lib/* /opt/voxer/lib/

# use ubuntu:20.04 if GPU is not used
FROM nvidia/opengl:1.2-glvnd-runtime-ubuntu20.04
COPY --from=builder /opt/voxer /opt/voxer
EXPOSE 3040

CMD ["/opt/voxer/bin/VoxerRemote"]
