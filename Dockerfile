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

# Patch ns/__init__.py to skip the netsimulyzer module (prevents SIGSEGV on import)
RUN python3 - <<'PYEOF'
import re, pathlib
init = pathlib.Path("/opt/ns-allinone-3.47/ns-3.47/build/bindings/python/ns/__init__.py")
src = init.read_text()
if "BROKEN_MODULES" not in src:
    patch = 'BROKEN_MODULES = {"netsimulyzer"}\n'
    src = re.sub(
        r'(for modname, modfile in _modules\.items\(\):)',
        patch + r'    if modname in BROKEN_MODULES:\n        continue\n    \1',
        src
    )
    init.write_text(src)
PYEOF

# Create sub-module shim files so "import ns.core" etc. work
RUN python3 - <<'PYEOF'
import pathlib
ns_dir = pathlib.Path("/opt/ns-allinone-3.47/ns-3.47/build/bindings/python/ns")
shims = ["core","network","wifi","mobility","internet","applications",
         "flow_monitor","netanim","csma","bridge","lte","point_to_point",
         "propagation","spectrum","stats","antenna","buildings","energy",
         "uan","wimax","mesh","nix_vector_routing","olsr","aodv","dsdv",
         "dsr","wave","sixlowpan","lr_wpan","zigbee","fd_net_device",
         "tap_bridge","virtual_net_device","traffic_control"]
template = 'import cppyy\ntry:\n    import ns as _ns_pkg\n    _ns3 = cppyy.gbl.ns3\nexcept Exception:\n    _ns3 = cppyy.gbl.ns3\n\ndef __getattr__(name):\n    return getattr(_ns3, name)\n'
for s in shims:
    p = ns_dir / f"{s}.py"
    if not p.exists():
        p.write_text(template)
PYEOF

WORKDIR /work
# Default command: open a shell with env ready
CMD ["/bin/bash"]
