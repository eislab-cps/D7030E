#!/usr/bin/env bash
set -euo pipefail
echo "=== ns-3.40 smoke test ==="

# Set env but don't die if the import check warns
source "$(dirname "$0")/setup_env.sh"

echo "[1/3] Check version file"
if [ -f "$NS3_DIR/VERSION" ]; then
  head -n1 "$NS3_DIR/VERSION" || true
else
  echo "WARN: VERSION file not found at $NS3_DIR/VERSION"
fi

echo "[2/3] Run tutorial example via ns3 wrapper"
cd "$NS3_DIR"
./ns3 run --no-build examples/tutorial/first.py

echo "[3/3] Minimal sim via direct import (cppyy)"
python3 - <<'PY'
from ns import ns
ns.core.Simulator.Stop(ns.core.Seconds(0.1))
ns.core.Simulator.Run()
ns.core.Simulator.Destroy()
print("Simulator minimal run: OK")
PY

echo "=== smoke test: DONE ==="
