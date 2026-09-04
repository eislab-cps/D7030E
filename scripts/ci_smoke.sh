#!/usr/bin/env bash
set -euo pipefail

repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$repo_dir/scripts/setup_env.sh"
echo "=== ns-3.47 C++ smoke test ==="
"$repo_dir/scripts/ns3-check.sh"
"$repo_dir/scripts/run_cpp.sh" "$repo_dir/Lab-00-Introduction/code/Lab0_Cpp_Hello.cc"
echo "=== smoke test: DONE ==="
