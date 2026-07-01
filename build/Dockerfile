FROM emscripten/emsdk:5.0.7 AS builder

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      autoconf automake libtool m4 bison flex gawk pkg-config \
      curl ca-certificates xz-utils \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

ENV CC_FOR_BUILD=gcc
ENV BUILD_CC=gcc

ARG GMP_VERSION=6.3.0
ENV GMP_SHA256=a3c2b80201b89e68616f4ad30bc66aee4927c3ce50e33929ca819d5c43538898
WORKDIR /src
RUN curl -fL --retry 3 -o gmp.tar.xz "https://ftp.gnu.org/gnu/gmp/gmp-${GMP_VERSION}.tar.xz" \
    && echo "${GMP_SHA256}  gmp.tar.xz" | sha256sum -c - \
    && tar -xf gmp.tar.xz && rm gmp.tar.xz
WORKDIR /src/gmp-${GMP_VERSION}
RUN emconfigure ./configure \
      --host=none \
      --disable-shared --enable-static \
      --disable-assembly \
      --enable-cxx=no \
      --prefix=/gmp-em \
    && emmake make -j"$(nproc)" \
    && emmake make install \
    && ls -lh /gmp-em/lib/libgmp.a

ARG VERSION=3.2
ENV VERSION=${VERSION}
ENV GC_SHA256=3bb48af46ced4779facf41fdc2ee60e4ccb86eaa99d010b36685315df39c2ee2
WORKDIR /src
RUN curl -fL --retry 3 -o gnucobol.tar.xz \
      "https://downloads.sourceforge.net/project/gnucobol/gnucobol/${VERSION}/gnucobol-${VERSION}.tar.xz" \
    && echo "${GC_SHA256}  gnucobol.tar.xz" | sha256sum -c - \
    && tar -xf gnucobol.tar.xz && rm gnucobol.tar.xz
WORKDIR /src/gnucobol-${VERSION}

RUN touch cobc/parser.c cobc/parser.h cobc/ppparse.c cobc/ppparse.h \
          cobc/pplex.c cobc/scanner.c 2>/dev/null || true

ENV CFLAGS="-O2"
RUN emconfigure ./configure \
      --host=wasm32-unknown-emscripten \
      --prefix=/out \
      --disable-shared --enable-static \
      --disable-nls --disable-dependency-tracking \
      --without-db --without-curses --without-json --with-xml2=no \
      --with-math=gmp \
      GMP_CFLAGS="-I/gmp-em/include" \
      GMP_LIBS="-L/gmp-em/lib -lgmp"

RUN emmake make -C lib       -j"$(nproc)" \
 && emmake make -C libcob    -j"$(nproc)" \
 && emmake make -C cobc      -j"$(nproc)" \
 && emmake make -C libcob    install \
 && emmake make -C config    install \
 && emmake make -C copy      install \
 && echo "=== installed runtime data ===" && ls -R /out/share/gnucobol | head -40 \
 && test -f /out/share/gnucobol/config/default.conf \
 && echo "=== cobc objects ===" && ls cobc/*.o

COPY js /src/js

ENV COBC_LDFLAGS="\
    -lnodefs.js -lworkerfs.js \
    --post-js /src/js/post.js \
    -sWASM_BIGINT=1 \
    -sALLOW_MEMORY_GROWTH=1 \
    -sINITIAL_MEMORY=134217728 \
    -sSTACK_SIZE=5242880 \
    -sEXIT_RUNTIME=0 \
    -sINVOKE_RUN=0 \
    -sFORCE_FILESYSTEM=1 \
    -sMODULARIZE=1 \
    -sEXPORT_ES6=1 \
    -sERROR_ON_UNDEFINED_SYMBOLS=0 \
    -sEXPORTED_RUNTIME_METHODS=callMain,FS,NODEFS,WORKERFS,ENV \
    -sINCOMING_MODULE_JS_API=noInitialRun,wasmBinary,locateFile,preRun,postRun,instantiateWasm,print,printErr,arguments \
"
RUN mkdir -p /dist \
 && em++ ${COBC_LDFLAGS} \
      cobc/*.o \
      lib/.libs/libsupport.a \
      libcob/.libs/libcob.a \
      /gmp-em/lib/libgmp.a \
      -lm \
      --preload-file /out/share/gnucobol@/out/share/gnucobol \
      -o /dist/cobc.mjs \
 && ls -lh /dist

RUN [ "$(head -c4 /dist/cobc.wasm | od -An -tx1 | tr -d ' \n')" = "0061736d" ] \
 && [ "$(stat -c%s /dist/cobc.wasm)" -gt 65536 ] \
 && [ -f /dist/cobc.data ]

FROM emscripten/emsdk:5.0.7 AS cob-sysroot

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
      autoconf automake libtool m4 bison flex gawk pkg-config \
      curl ca-certificates xz-utils \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

RUN arch="$(uname -m)"; case "$arch" in \
      x86_64)        sdkarch=x86_64; sha=0ba8b5bfaeb2adf3f29bab5841d76cf5318ab8e1642ea195f88baba1abd47bce ;; \
      aarch64|arm64) sdkarch=arm64;  sha=4f98ee738c7abb45c81a94d1461fc53cc569d1cd01498951c8184d841a027844 ;; \
      *) echo "unsupported arch: $arch" >&2; exit 1 ;; \
    esac \
 && curl -fL --retry 3 -o /tmp/wasi-sdk.tar.gz \
      "https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-33/wasi-sdk-33.0-${sdkarch}-linux.tar.gz" \
 && echo "${sha}  /tmp/wasi-sdk.tar.gz" | sha256sum -c - \
 && mkdir -p /opt/wasi-sdk \
 && tar -xz --strip-components=1 -C /opt/wasi-sdk -f /tmp/wasi-sdk.tar.gz \
 && rm /tmp/wasi-sdk.tar.gz \
 && /opt/wasi-sdk/bin/clang --version

ENV WCC="/opt/wasi-sdk/bin/clang"
ENV AR=/opt/wasi-sdk/bin/llvm-ar
ENV RANLIB=/opt/wasi-sdk/bin/llvm-ranlib
ENV CC_FOR_BUILD=gcc
ENV BUILD_CC=gcc
ENV COB_SYSROOT=/cob-sysroot

ENV WASI_EMU="-D_WASI_EMULATED_SIGNAL -D_WASI_EMULATED_MMAN -D_WASI_EMULATED_PROCESS_CLOCKS -D_WASI_EMULATED_GETPID"
ENV WASI_EMU_LIBS="-lwasi-emulated-signal -lwasi-emulated-mman -lwasi-emulated-process-clocks -lwasi-emulated-getpid"

ARG GMP_VERSION=6.3.0
ENV GMP_SHA256=a3c2b80201b89e68616f4ad30bc66aee4927c3ce50e33929ca819d5c43538898
WORKDIR /src
RUN curl -fL --retry 3 -o gmp.tar.xz "https://ftp.gnu.org/gnu/gmp/gmp-${GMP_VERSION}.tar.xz" \
 && echo "${GMP_SHA256}  gmp.tar.xz" | sha256sum -c - \
 && tar -xf gmp.tar.xz && rm gmp.tar.xz
WORKDIR /src/gmp-${GMP_VERSION}
RUN ./configure \
      --host=none \
      --disable-shared --enable-static --disable-assembly --enable-cxx=no \
      --prefix="${COB_SYSROOT}" \
      CC="${WCC}" AR="${AR}" RANLIB="${RANLIB}" \
      CFLAGS="-O2 ${WASI_EMU} -Wno-error=implicit-function-declaration" \
    && make -j"$(nproc)" \
    && make install \
    && ls -lh "${COB_SYSROOT}/lib/libgmp.a"

ARG VERSION=3.2
ENV VERSION=${VERSION}
ENV GC_SHA256=3bb48af46ced4779facf41fdc2ee60e4ccb86eaa99d010b36685315df39c2ee2
WORKDIR /src
RUN curl -fL --retry 3 -o gnucobol.tar.xz \
      "https://downloads.sourceforge.net/project/gnucobol/gnucobol/${VERSION}/gnucobol-${VERSION}.tar.xz" \
    && echo "${GC_SHA256}  gnucobol.tar.xz" | sha256sum -c - \
    && tar -xf gnucobol.tar.xz && rm gnucobol.tar.xz
WORKDIR /src/gnucobol-${VERSION}
RUN touch cobc/parser.c cobc/parser.h cobc/ppparse.c cobc/ppparse.h \
          cobc/pplex.c cobc/scanner.c 2>/dev/null || true

RUN ./configure \
      --host=wasm32-unknown-wasi \
      --prefix="${COB_SYSROOT}" \
      --disable-shared --enable-static \
      --disable-nls --disable-dependency-tracking \
      --without-db --without-curses --without-json --with-xml2=no \
      --with-math=gmp \
      ac_cv_func_fcntl=no \
      CC="${WCC}" AR="${AR}" RANLIB="${RANLIB}" \
      CFLAGS="-O2 ${WASI_EMU} -mllvm -wasm-enable-sjlj -Wno-error=implicit-function-declaration" \
      GMP_CFLAGS="-I${COB_SYSROOT}/include" \
      GMP_LIBS="-L${COB_SYSROOT}/lib -lgmp ${WASI_EMU_LIBS}"

RUN make -C lib    -j"$(nproc)" \
 && make -C libcob -j"$(nproc)" \
 && make -C libcob install \
 && make install-includeHEADERS \
 && cp $(find lib -name 'libsupport.a' | head -1) "${COB_SYSROOT}/lib/libsupport.a"

COPY deps /src/deps
RUN ${WCC} -O2 -c /src/deps/wasm-stubs.c -o /tmp/wasm-stubs.o \
 && "${AR}" rcs "${COB_SYSROOT}/lib/libcobstubs.a" /tmp/wasm-stubs.o \
 && cp /opt/wasi-sdk/share/wasi-sysroot/lib/wasm32-wasi/libsetjmp.a "${COB_SYSROOT}/lib/libsetjmp.a" \
 && echo "=== cob-sysroot ===" && ls -lhR "${COB_SYSROOT}/lib" "${COB_SYSROOT}/include" | head -60

RUN mkdir -p /dist \
 && for a in libcob libgmp libsupport libcobstubs libsetjmp; do \
      "${AR}" t "${COB_SYSROOT}/lib/$a.a" >/dev/null || exit 1; \
    done \
 && tar -czf /dist/libcob-wasi.tar.gz -C "${COB_SYSROOT}" include lib \
 && ls -lh /dist/libcob-wasi.tar.gz

FROM scratch AS exportor
COPY --from=builder     /dist/cobc.mjs        /dist/cobc.mjs
COPY --from=builder     /dist/cobc.wasm       /dist/cobc.wasm
COPY --from=builder     /dist/cobc.data       /dist/cobc.data
COPY --from=cob-sysroot /dist/libcob-wasi.tar.gz /dist/libcob-wasi.tar.gz
