#!/usr/bin/env bash
# Makes starfox64.us.rev1.uncompressed.z64 in the repository's root, the ROM N64Recomp reads (us.rev1.toml), from a
# dump of the US 1.1 ROM: the Star Fox 64 decompilation's comptool.py with its MIO0 decompressor, built here with the
# host's C compiler ($CC, cc without), and the result checked against the decompilation's md5. The program itself,
# native or the page, takes the plain dump and decompresses it in memory.
#
#   bash tools/decompress_rom.sh <US 1.1 ROM>
set -euo pipefail
[ $# -eq 1 ] || { sed -n '7p' "$0"; exit 1; }
rom="$(realpath "$1")"
cd "$(dirname "$0")/.."
decomp=lib/sf64decomp
t="$(mktemp -d)"
trap 'rm -rf "$t"' EXIT
"${CC:-cc}" -O2 -DMIO0_STANDALONE -o "$t/mio0" "$decomp/tools/mio0-decompressor/libmio0.c"
python3 -I "$decomp/tools/comptool.py" -d -m "$t/mio0" "$rom" "$t/out.z64" > /dev/null
want="$(cut -c1-32 "$decomp/starfox64.us.rev1.uncompressed.md5")"
got="$(md5sum "$t/out.z64" | cut -c1-32)"
[[ "$got" == "$want" ]] || { echo "decompressed md5 $got, not $want: is it the US 1.1 ROM?"; exit 1; }
mv "$t/out.z64" starfox64.us.rev1.uncompressed.z64
echo "starfox64.us.rev1.uncompressed.z64 ($got)"
