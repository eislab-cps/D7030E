#!/usr/bin/env bash
set -euo pipefail
# Packages the repo as a clean zip without .git/, build artifacts, or large files.
out="${1:-advanceWirelessNetworks_release.zip}"
tmpdir="$(mktemp -d)"
trap 'rm -rf "$tmpdir"' EXIT
rsync -a --exclude '.git' --exclude '.github/workflows/*.cache' --exclude '__pycache__' \
      --exclude 'build' --exclude 'cmake-build*' --exclude '*.o' --exclude '*.so' \
      ./ "$tmpdir/repo/"
( cd "$tmpdir/repo" && zip -r "../$out" . )
mv "$tmpdir/$out" ./
echo "Wrote $out"