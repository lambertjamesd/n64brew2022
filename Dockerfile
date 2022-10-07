from ubuntu:20.04

WORKDIR /usr/src/app

ENV N64_LIBGCCDIR /opt/crashsdk/lib/gcc/mips64-elf/11.2.0
ENV PATH /opt/crashsdk/bin:$PATH
ENV ROOT /etc/n64

RUN apt update -y --fix-missing
RUN apt install -y ca-certificates

RUN echo "deb [trusted=yes] https://crashoveride95.github.io/apt/ ./" | tee /etc/apt/sources.list.d/n64sdk.list
RUN apt update -y

RUN dpkg --add-architecture i386

ENV TZ=America/Denver
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt install -y binutils-mips-n64 \
    gcc-mips-n64 \
    n64sdk \
    libnustd \
    makemask \
    root-compatibility-environment \
    build-essential \
    libmpc-dev \
    libxi6 \
    libxxf86vm-dev \
    libxfixes3 \
    libxrender1 \
    libgl1 \
    python3 \
    nodejs \
    imagemagick \
    libpng-dev \
    libtiff-dev \
    libassimp-dev \
    unzip \
    build-essential \
    luarocks

COPY Makefile Makefile
COPY asm asm
COPY assets assets
COPY src src
COPY tools tools
COPY game.ld game.ld

RUN luarocks install --tree lua_modules lunajson

CMD make