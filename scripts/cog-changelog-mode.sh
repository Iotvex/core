#!/usr/bin/env bash
# Toggle Cocogitto changelog writes in cog.toml (stable main releases only).
# Usage: ./scripts/cog-changelog-mode.sh enable|disable
set -euo pipefail

MODE="${1:?usage: cog-changelog-mode.sh <enable|disable>}"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
COG="$ROOT/cog.toml"

case "$MODE" in
  enable)
  sed -i 's/^disable_changelog = true/disable_changelog = false/' "$COG"
  ;;
  disable)
  if grep -q '^disable_changelog' "$COG"; then
    sed -i 's/^disable_changelog = false/disable_changelog = true/' "$COG"
  else
    sed -i '/^from_latest_tag = true/a disable_changelog = true' "$COG"
  fi
  ;;
  *)
  echo "error: mode must be enable or disable" >&2
  exit 1
  ;;
esac
