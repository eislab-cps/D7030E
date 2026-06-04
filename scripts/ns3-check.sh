#!/usr/bin/env bash
set -euo pipefail
NS3_DIR="${NS3_DIR:-/opt/ns-allinone-3.40/ns-3.40}"
if [ ! -f "$NS3_DIR/VERSION" ]; then
  echo "ns-3 VERSION file not found at $NS3_DIR/VERSION"
  exit 1
fi
ver=$(cat "$NS3_DIR/VERSION" | tr -d '\n')
echo "Detected: $ver"
case "$ver" in
  *"ns-3.40"*) echo "OK: pinned to ns-3.40";;
  *) echo "WARNING: not ns-3.40"; exit 2;;
esac