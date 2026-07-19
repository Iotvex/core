#!/usr/bin/env bash
# Sync SemVer into ESP-IDF / PlatformIO manifests (and README).
# Usage: ./scripts/sync-version.sh <version>
# Accepts 0.2.0 or v0.2.0
set -euo pipefail

RAW="${1:?usage: sync-version.sh <version>}"
VERSION="${RAW#v}"

if [[ ! "$VERSION" =~ ^[0-9]+\.[0-9]+\.[0-9]+([.-].*)?$ ]]; then
  echo "error: invalid version '$RAW'" >&2
  exit 1
fi

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

if [[ ! -f idf_component.yml || ! -f library.json ]]; then
  echo "error: run from IotvexCore repo root (missing manifests)" >&2
  exit 1
fi

PYTHON=""
if command -v python3 >/dev/null 2>&1 && python3 -c "import sys" >/dev/null 2>&1; then
  PYTHON="python3"
elif command -v python >/dev/null 2>&1 && python -c "import sys" >/dev/null 2>&1; then
  PYTHON="python"
else
  echo "error: python3/python required" >&2
  exit 1
fi

"$PYTHON" - "$VERSION" <<'PY'
import json
import re
import sys
from pathlib import Path

version = sys.argv[1]

idf = Path("idf_component.yml")
text = idf.read_text(encoding="utf-8")
new_text, count = re.subn(
    r'(?m)^version:\s*["\']?[^"\'\n]+["\']?',
    f'version: "{version}"',
    text,
    count=1,
)
if count != 1:
    raise SystemExit("error: could not update version in idf_component.yml")
idf.write_text(new_text, encoding="utf-8")

lib = Path("library.json")
data = json.loads(lib.read_text(encoding="utf-8"))
data["version"] = version
lib.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")

readme = Path("README.md")
if readme.exists():
    readme_text = readme.read_text(encoding="utf-8")
    readme_new, n = re.subn(
        r"(?m)^\*\*Version:\*\*\s*`[^`]+`",
        f"**Version:** `{version}`",
        readme_text,
        count=1,
    )
    if n:
        readme.write_text(readme_new, encoding="utf-8")

print(f"synced version -> {version}")
PY
