# Building

The game's code is recompiled at build time from your own dump of Star Fox 64 (US 1.1, Rev A, big-endian `.z64`, md5
741a94eee093c4c8684e66b89f8685e8), decompressed first. The program, native or the page, then takes the plain dump from
its launcher's Load ROM and decompresses it in memory.

## 1. Clone with the submodules

```bash
git clone --recurse-submodules https://github.com/n64hub/Starfox64Recomp-web
```

`lib/` holds RT64, N64ModernRuntime (with N64Recomp) and RecompFrontend from their upstream repositories, at the
commits [N64Recomp-browser](https://github.com/n64hub/N64Recomp-browser)'s web forks are built on
(Starfox64Recomp itself predates RecompFrontend, so its own pins are older); N64Recomp-browser, whose `patches/`
puts the web support into their checkouts and whose `frontend/` makes the page; and the Star Fox 64 decompilation
(`lib/sf64decomp`, whose headers the patches use and whose tools decompress the ROM). `Starfox64RecompSyms` holds the
symbols N64Recomp reads.

## 2. Tools

- CMake 3.20+, Ninja, clang for the program (clang-cl on Windows), SDL2 and GTK 3 on Linux
  (`sudo apt-get install cmake ninja-build libsdl2-dev libgtk-3-dev lld llvm clang`).
- A clang with a MIPS target and `ld.lld`, plus `make`, for the patches (`patches/`). Linux's clang has one. On Windows
  llvm.org's clang has none: MSYS2's clang64 does, with llvm.org's `ld.lld`.
- Python 3 and a C compiler, to decompress the ROM.
- For the page: emsdk (`emsdk install latest`) and [naga](https://github.com/gfx-rs/wgpu/tree/trunk/naga)
  (`cargo install naga-cli`).

## 3. The recompiled code

```bash
# starfox64.us.rev1.uncompressed.z64, which N64Recomp reads
bash tools/decompress_rom.sh <your US 1.1 dump>

# N64Recomp-browser's patches in lib/rt64, lib/N64ModernRuntime and lib/RecompFrontend
cmake -P lib/N64Recomp-browser/patches/apply.cmake lib

# N64Recomp and RSPRecomp, from the patched N64Recomp in lib/
cmake -S lib/N64ModernRuntime/N64Recomp -B build-n64recomp -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-n64recomp --target N64RecompCLI RSPRecomp
cp build-n64recomp/N64Recomp build-n64recomp/RSPRecomp .                  # .exe on Windows

./N64Recomp us.rev1.toml            # RecompiledFuncs/
./RSPRecomp aspMain.us.rev1.toml    # rsp/aspMain.cpp
```

The native program needs the patches as much as the page does: the code the patched N64Recomp writes uses the patched
`recomp.h` (`RECOMP_TAIL_CALL`). The web support in them is behind `__EMSCRIPTEN__` and changes nothing natively.
Configuring the program (step 4, or the page) applies them too if they aren't in yet, and does nothing if they are;
`git apply --reverse lib/N64Recomp-browser/patches/<name>.patch` in a checkout takes one out.

The build compiles and recompiles the patches itself (`make` in `patches/`, then `./N64Recomp patches.toml`).

## 4. Native

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
    -DPATCHES_C_COMPILER=<clang with a MIPS target> -DPATCHES_LD=<ld.lld>    # clang-cl for the program on Windows
cmake --build build --target Starfox64Recompiled
```

Run it from the repository's root (it reads `assets/`). A `portable.txt` there keeps settings, saves and the stored
ROM beside it.

## The web page

```bash
embuilder build sdl2-mt freetype-legacysjlj      # once
emcmake cmake -S . -B build-web -G Ninja -DCMAKE_BUILD_TYPE=Release -DRT64_NAGA_EXECUTABLE=<naga> \
    -DPATCHES_C_COMPILER=<clang with a MIPS target> -DPATCHES_LD=<ld.lld>
cmake --build build-web --target Starfox64Recompiled
```

Keep `build-web` inside the checkout and put [ccache](https://ccache.dev) on `PATH`: N64Recomp-browser's
`frontend/build_speed.cmake` puts it in front of every compile, and a second fresh build dir then compiles almost
nothing. emsdk's clang has no MIPS target, so the patches are compiled with the host's clang (`PATCHES_C_COMPILER`).

`build-web/Starfox64Recompiled.html` must be served with `Cross-Origin-Opener-Policy: same-origin` and
`Cross-Origin-Embedder-Policy: require-corp` (threads need SharedArrayBuffer), or with `coi_serviceworker.js` beside
it where headers can't be set. Open it in a browser with WebGPU and pick your dump with Load ROM: it is kept in the
browser's storage, with settings and saves.

### The shader list

The page specializes the shaders in `src/rt64_shaders.txt` at build time (`RT64_PREBUILT_SHADER_LIST`), so that it
never draws with RT64's slow general shader. The list is recorded by `tools/record_shaders.sh <build-web> <rom>`, which
plays `tools/shader_plan.json` with N64Recomp-browser's `tools/record_shaders.mjs`: the page opened with
`?record-shaders` prints every shader the game asks for, and the plan reaches each level by code
(`tools/shader_survey.js` and `patches/shader_survey.c`, which start a level through the game's own change of state,
with the player kept alive and firing). Its header says what was played and what wasn't.

`tools/play_plan.json` is the page's regular play, 2-3 minutes with every timeout counted, which N64Recomp-browser's
`tools/play_page.mjs` plays to measure the page: the menus and the Lylat map to Corneria, flown a minute with A held.
`tools/play_plan_extended.json`, at most 5 minutes, goes on to Meteo and Katina by the survey's `level()`, for when a
longer play is needed, and the shaders it misses go into the list. Each plan's header says what it plays.

## What was played

Not yet.
