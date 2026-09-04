#!/usr/bin/env bash
set -euo pipefail

NS3_DIR="${NS3_DIR:-$HOME/ns-allinone-3.47/ns-3.47}"
if [[ ! -f "$NS3_DIR/VERSION" ]]; then
  echo "ns-3 VERSION file not found at $NS3_DIR/VERSION" >&2
  exit 1
fi
version="$(tr -d '\n' < "$NS3_DIR/VERSION")"
echo "Detected: $version"
case "$version" in
  *3.47*) echo "OK: pinned to ns-3.47" ;;
  *) echo "ERROR: expected ns-3.47" >&2; exit 2 ;;
esac
"$NS3_DIR/ns3" build
