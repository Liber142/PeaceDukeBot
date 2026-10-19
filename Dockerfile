FROM ubuntu:24.04 AS build
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
    g++ cmake ninja-build pkg-config git python3 ca-certificates \
    libssl-dev zlib1g-dev libsodium-dev libopus-dev \
    libcurl4-openssl-dev libtbb-dev libmpg123-dev \
 && rm -rf /var/lib/apt/lists/*

ARG REPLXX_VERSION=release-0.0.4
RUN git clone --depth 1 --branch ${REPLXX_VERSION} https://github.com/AmokHuginnsson/replxx.git /tmp/replxx \
 && cmake -S /tmp/replxx -B /tmp/replxx/build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release -DREPLXX_BUILD_EXAMPLES=OFF \
 && cmake --build /tmp/replxx/build \
 && cmake --install /tmp/replxx/build \
 && rm -rf /tmp/replxx

ARG CPR_VERSION=1.10.5
RUN git clone --depth 1 --branch ${CPR_VERSION} https://github.com/libcpr/cpr.git /tmp/cpr \
 && cmake -S /tmp/cpr -B /tmp/cpr/build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCPR_USE_SYSTEM_CURL=ON -DCPR_BUILD_TESTS=OFF \
 && cmake --build /tmp/cpr/build \
 && cmake --install /tmp/cpr/build \
 && rm -rf /tmp/cpr

ARG DPP_REF=v10.1.4
RUN git clone https://github.com/brainboxdotcc/DPP.git /tmp/dpp \
 && git -C /tmp/dpp checkout ${DPP_REF} \
 && cmake -S /tmp/dpp -B /tmp/dpp/build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release -DDPP_BUILD_TEST=OFF \
 && cmake --build /tmp/dpp/build \
 && cmake --install /tmp/dpp/build \
 && rm -rf /tmp/dpp

WORKDIR /src
COPY . .

ARG WT=docker
COPY --from=gitdir . /gitdir
ENV GIT_DIR=/gitdir/worktrees/${WT}

RUN cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build --target discord-bot \
 && strip build/discord-bot

FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
    ca-certificates libssl3t64 zlib1g libsodium23 libopus0 \
    libcurl4t64 libtbb12 libmpg123-0t64 \
 && rm -rf /var/lib/apt/lists/*

COPY --from=build /usr/local/lib/ /usr/local/lib/
RUN ldconfig

RUN useradd --system --uid 10001 --user-group --no-create-home bot \
 && mkdir -p /PeaceDukeBot/data \
 && chown -R bot:bot /PeaceDukeBot

WORKDIR /PeaceDukeBot
COPY --from=build --chmod=755 /src/build/discord-bot /PeaceDukeBot/discord-bot

RUN ! ldd /PeaceDukeBot/discord-bot | grep "not found"

USER bot
CMD ["/PeaceDukeBot/discord-bot"]
