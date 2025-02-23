FROM ubuntu:22.04 AS base

FROM base as build

# install build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake=3.22.1-1ubuntu1.22.04.2 \
    ninja-build=1.10.1-1 \
    gcc-12=12.3.0-1ubuntu1~22.04 \
    g++-12=12.3.0-1ubuntu1~22.04 \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# tell cmake where the compilers are
ENV CC=/usr/bin/gcc-12 CXX=/usr/bin/g++-12

WORKDIR /app
# copy data
COPY games.txt .
# copy source code
COPY src /app/src

# build
RUN cmake -G Ninja -S src -B build && ninja -C build

FROM base AS prod
WORKDIR /app
COPY --from=build /app/games.txt .
COPY --from=build /app/build/sc3020_p1 .
CMD ["./sc3020_p1", "games.txt"]
