#!/usr/bin/env bash
set -euo pipefail
NS3_DIR="${NS3_DIR:-/opt/ns-allinone-3.47/ns-3.47}"
if [ ! -f "$NS3_DIR/VERSION" ]; then
  echo "ns-3 VERSION file not found at $NS3_DIR/VERSION"
  exit 1
fi
ver=$(cat "$NS3_DIR/VERSION" | tr -d '\n')
echo "Detected: $ver"
case "$ver" in
  *"3.47"*) echo "OK: pinned to ns-3.47";;
  *) echo "WARNING: not ns-3.47"; exit 2;;
esac
