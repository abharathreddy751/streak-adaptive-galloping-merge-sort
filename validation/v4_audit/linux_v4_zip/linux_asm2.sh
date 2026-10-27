#!/usr/bin/env bash
W="/mnt/d/new dsa project_V4_WORK"
rm -rf /tmp/asmwork && mkdir -p /tmp/asmwork && cp -r "$W/src" "$W/validation" /tmp/asmwork/ && cd /tmp/asmwork
mkdir -p scripts && cp "$W/scripts/package_submission.py" scripts/ 2>/dev/null
mkdir -p /tmp/pyshim && ln -sf "$(command -v python3)" /tmp/pyshim/python
echo "g++ $(g++ --version | head -1)"
PATH="/tmp/pyshim:$PATH" bash validation/v4/asm_check/asm_check.sh
echo "asm_check exit=$?"
