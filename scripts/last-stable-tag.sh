#!/usr/bin/env bash
# Print the latest stable (non pre-release) SemVer tag reachable from HEAD.
set -euo pipefail

git fetch --tags --force origin >/dev/null 2>&1 || true

git tag --merged HEAD --list 'v*' \
  | grep -Ev -- '-(dev|alpha|beta|rc)(\.|$)' \
  | sort -V \
  | tail -n1
