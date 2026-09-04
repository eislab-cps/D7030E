#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
  echo "Usage: scripts/run_cpp.sh path/to/program.cc [simulation arguments...]" >&2
  exit 2
fi

repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
source "$repo_dir/scripts/setup_env.sh"

source_file="$1"
shift
if [[ "$source_file" != /* ]]; then source_file="$repo_dir/$source_file"; fi
if [[ ! -f "$source_file" || "$source_file" != *.cc ]]; then
  echo "C++ source not found: $source_file" >&2
  exit 2
fi

# Stage the selected program in the repo's exec/ folder (gitignored), the
# same place the Windows workflow has always kept the "current" .cc file.
exec_dir="$repo_dir/exec"
find "$exec_dir" -maxdepth 1 -type f -name '*.cc' -delete
cp "$source_file" "$exec_dir/"

# Mirror exec/ into ns-3's scratch/ so its CMakeLists.txt (a single program
# always named "exec") gets built and run.
scratch_dir="$NS3_DIR/scratch/exec"
mkdir -p "$scratch_dir"
find "$scratch_dir" -maxdepth 1 -type f -name '*.cc' -delete
cp "$exec_dir/CMakeLists.txt" "$scratch_dir/CMakeLists.txt"
cp "$exec_dir"/*.cc "$scratch_dir/"

cd "$NS3_DIR"
./ns3 build
./ns3 run exec -- "$@"
