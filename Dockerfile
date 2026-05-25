# ── Stage 1: builder ──────────────────────────────────────────────
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libmysqlclient-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .

# CMakeLists.txt currently applies -O0 -g unconditionally.
# When the debug flags become build-type conditional, switch to Release.
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug \
    && cmake --build build --target game_server -j$(nproc)

# ── Stage 2: runtime ──────────────────────────────────────────────
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libmysqlclient21 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /src/build/game_server .
COPY --from=builder /src/config/            config/

EXPOSE 12345

CMD ["./game_server"]
