#!/usr/bin/env bash
set -euo pipefail

export NS3_VERSION="${NS3_VERSION:-3.47}"
export NS3_ALLINONE="${NS3_ALLINONE:-$HOME/ns-allinone-${NS3_VERSION}}"
export NS3_DIR="${NS3_DIR:-$NS3_ALLINONE/ns-${NS3_VERSION}}"

if [[ ! -x "$NS3_DIR/ns3" ]]; then
  echo "ns-3.47 was not found at $NS3_DIR" >&2
  echo "Run scripts/install_wsl.sh first." >&2
  return 1 2>/dev/null || exit 1
fi

export LD_LIBRARY_PATH="$NS3_DIR/build/lib:${LD_LIBRARY_PATH:-}"
export PATH="$NS3_DIR:$NS3_DIR/build:${PATH:-}"
echo "[ns3-env] NS3_DIR=$NS3_DIR"
