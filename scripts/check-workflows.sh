#!/usr/bin/env bash
set -euo pipefail

fail=0
echo "Checking GitHub workflow YAML files..."
shopt -s nullglob
for f in .github/workflows/*.yml .github/workflows/*.yaml; do
  echo "  - Validating $f"
  if ! python3 - <<PY
import sys,yaml
try:
    with open('$f','r') as fh:
        yaml.safe_load(fh)
except Exception as e:
    print('YAML parse error in: ' + '$f')
    print(e)
    sys.exit(1)
PY
  then
    echo "    -> INVALID: $f"
    fail=1
  else
    echo "    -> OK"
  fi
done

if [ "$fail" -ne 0 ]; then
  echo "One or more workflow YAML files are invalid. See errors above." >&2
  exit 1
fi

echo "All workflow YAML files look valid."
