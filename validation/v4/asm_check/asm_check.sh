#!/usr/bin/env bash
# Shows that the v4 trace hook does not change the code generated for the uncounted (timed) sorters:
# compiles asm_instances.cpp (Count=false instantiations) against the v3 header (orig_hdr/) and the v4 header (src/) and compares the assembly
# function by function (asm_compare.py: emission order and local label numbers are normalised, everything else must be identical).
set -u
cd "$(dirname "$0")/../../.." || exit 2
if ! command -v g++ >/dev/null 2>&1 && [ -x /c/msys64/usr/bin/g++ ]; then export PATH="/c/msys64/usr/bin:$PATH"; fi
D=validation/v4/asm_check
for flags in "-O2"; do  # -O2 is what scripts/build.sh and the benchmarks use
  g++ -std=c++17 $flags -S -fno-asynchronous-unwind-tables -I$D/orig_hdr $D/asm_instances.cpp -o $D/asm_v3_${flags#-}.s || exit 1
  g++ -std=c++17 $flags -S -fno-asynchronous-unwind-tables -Isrc $D/asm_instances.cpp -o $D/asm_v4_${flags#-}.s || exit 1
  python -I $D/asm_compare.py $D/asm_v3_${flags#-}.s $D/asm_v4_${flags#-}.s || { echo "asm DIFFERS at $flags"; exit 1; }
  echo "asm identical (function by function) at $flags"
done
echo "ASM CHECK: PASS"
