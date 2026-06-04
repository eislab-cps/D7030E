#!/usr/bin/env bash
set -euo pipefail

export NS3_VERSION="${NS3_VERSION:-3.47}"
export NS3_ALLINONE="/opt/ns-allinone-${NS3_VERSION}"
# Correct layout: /opt/ns-allinone-3.47/ns-3.47
export NS3_DIR="${NS3_ALLINONE}/ns-${NS3_VERSION}"

# Fallback autodetect if layout changes
if [ ! -d "$NS3_DIR" ]; then
  cand=$(ls -d "$NS3_ALLINONE"/ns-* 2>/dev/null | head -n1 || true)
  [ -n "${cand:-}" ] && NS3_DIR="$cand"
fi
export NS3_DIR

# Export envs (don’t rely on wrapper for simple imports)
export LD_LIBRARY_PATH="${NS3_DIR}/build/lib:${LD_LIBRARY_PATH:-}"
export PYTHONPATH="${NS3_DIR}/build/bindings/python:${PYTHONPATH:-}"
export PATH="${NS3_DIR}:${NS3_DIR}/build:${PATH:-}"

echo "[ns3-env] NS3_DIR=$NS3_DIR"

# Try new-style cppyy import; never exit here (we just warn)
python3 - <<'PY' || true
try:
    from ns import ns
    print("[ns3-env] Python 'ns' import: OK")
except Exception as e:
    print("[ns3-env] Python 'ns' import WARNING:", e)
PY
