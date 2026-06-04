#!/usr/bin/env bash
# Utility: seed_manager
# Usage: seed_manager.sh <num_runs> <executable> [args...]

if [ "$#" -lt 2 ]; then
  echo "Usage: $0 <num_runs> <executable> [args...]"
  exit 1
fi

NUM_RUNS=$1
EXEC=$2
shift 2

for ((i=1; i<=NUM_RUNS; i++)); do
  echo "=== Run seed=$i ==="
  $EXEC --seed $i "$@"
done