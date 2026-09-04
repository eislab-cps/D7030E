#!/usr/bin/env bash
set -euo pipefail

if ! grep -qi microsoft /proc/version; then
  echo "This installer is intended for Ubuntu on WSL2." >&2
  exit 1
fi

version="3.47"
allinone="$HOME/ns-allinone-$version"
archive="$HOME/ns-allinone-$version.tar.bz2"
as_root=()
if [[ $EUID -ne 0 ]]; then as_root=(sudo); fi

"${as_root[@]}" apt-get update
DEBIAN_FRONTEND=noninteractive "${as_root[@]}" apt-get install -y \
  build-essential gcc g++ cmake ninja-build git curl pkg-config \
  autoconf automake libxmu-dev p7zip-full \
  qtbase5-dev qttools5-dev-tools \
  gnuplot-x11 wireshark net-tools

if [[ ! -d "$allinone/ns-$version" ]]; then
  curl -fL "https://www.nsnam.org/releases/ns-allinone-$version.tar.bz2" -o "$archive"
  tar xjf "$archive" -C "$HOME"
fi

cd "$allinone/ns-$version"
./ns3 configure --enable-examples --enable-tests
./ns3 build
echo "ns-3.47 is ready at $allinone/ns-$version"
