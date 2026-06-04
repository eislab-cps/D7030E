# ns-3.47 environment for D7030E Advanced Wireless Networks labs
FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive
ARG NS3_VERSION=3.47
ARG BUILD_NETANIM=false

# Base build deps
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    cmake \
    curl \
    git \
    pkg-config \
    python3 python3-dev python3-pip \
    libxml2 libxml2-dev \
    ninja-build \
    qtbase5-dev qtchooser qt5-qmake \
    && rm -rf /var/lib/apt/lists/*

# Fetch and build ns-3 allinone for the pinned version
WORKDIR /opt
RUN curl -fsSL https://www.nsnam.org/release/ns-allinone-${NS3_VERSION}.tar.bz2 -o ns-allinone-${NS3_VERSION}.tar.bz2 && \
    tar -xjf ns-allinone-${NS3_VERSION}.tar.bz2 && \
    rm ns-allinone-${NS3_VERSION}.tar.bz2 && \
    cd ns-allinone-${NS3_VERSION} && \
    ./build.py --enable-examples --enable-tests

# Environment for Python + shared libs
ENV NS3_ALLINONE=/opt/ns-allinone-${NS3_VERSION}
ENV NS3_DIR=${NS3_ALLINONE}/ns-${NS3_VERSION}
ENV LD_LIBRARY_PATH=${NS3_DIR}/build/lib
ENV PYTHONPATH=${NS3_DIR}/build/bindings/python
ENV PATH=${NS3_DIR}:${NS3_DIR}/build:${PATH}

# Optional NetAnim build (GUI usage requires X-forwarding on host)
RUN if [ "$BUILD_NETANIM" = "true" ]; then \
      cd $NS3_ALLINONE/netanim-* && qmake && make -j$(nproc); \
    fi

# Common Python tools + required cppyy for Python bindings (ns-3.47 uses Cppyy)
RUN pip3 install --no-cache-dir matplotlib pandas lxml cppyy

# Ensure Python bindings are ON and built
RUN cd ${NS3_DIR} \
&& ./ns3 configure --enable-examples --enable-tests --enable-python-bindings \
&& CMAKE_BUILD_PARALLEL_LEVEL=$(nproc) ./ns3 build

WORKDIR /work
# Default command: open a shell with env ready
CMD ["/bin/bash"]