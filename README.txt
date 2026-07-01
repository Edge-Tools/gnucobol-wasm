gnucobol-wasm — Corresponding Source mirror
==================================================

This repository publishes the Corresponding Source for the WebAssembly
build of gnucobol (license: GPL-3.0-or-later) used in edgetools.io.

Contents
  build/      our build recipe: Dockerfile + helper scripts/config/patches.
              Rebuild with:  docker build build/
  upstream/   the exact upstream source archive(s) the build fetched,
              byte-identical and sha256-verified (see below).

Upstream sources:
  gnucobol.tar.xz
    https://downloads.sourceforge.net/project/gnucobol/gnucobol/3.2/gnucobol-3.2.tar.xz
    sha256 3bb48af46ced4779facf41fdc2ee60e4ccb86eaa99d010b36685315df39c2ee2
  gmp.tar.xz
    https://ftp.gnu.org/gnu/gmp/gmp-6.3.0.tar.xz
    sha256 a3c2b80201b89e68616f4ad30bc66aee4927c3ce50e33929ca819d5c43538898
