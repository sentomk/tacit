FROM debian:trixie-slim

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       build-essential clang clang-format clang-tidy gdb \
       cmake ninja-build git ca-certificates python3 python3-venv \
    && rm -rf /var/lib/apt/lists/* \
    && useradd --create-home --shell /bin/bash dev

USER dev
