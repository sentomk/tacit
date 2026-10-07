FROM debian:trixie-slim

# micromamba creates the pinned Sage environment (environment.yml) on demand. Only this
# small static binary is baked into the image; Sage itself is not, because it is several
# GB and only needed to regenerate tests/vectors. The checksum matches the upstream
# release's published .sha256 file.
ARG MICROMAMBA_VERSION=2.9.0-0
ARG MICROMAMBA_SHA256=366cd9cd8be14df1ab8ed50352a82111082a36686b2d389fdb79a92c3fafb3e3

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       build-essential clang clang-format clang-tidy gdb \
       cmake ninja-build git ca-certificates curl python3 python3-venv \
    && rm -rf /var/lib/apt/lists/* \
    && useradd --create-home --shell /bin/bash dev

RUN curl -fsSL -o /usr/local/bin/micromamba \
       "https://github.com/mamba-org/micromamba-releases/releases/download/${MICROMAMBA_VERSION}/micromamba-linux-64" \
    && echo "${MICROMAMBA_SHA256}  /usr/local/bin/micromamba" | sha256sum -c - \
    && chmod 0755 /usr/local/bin/micromamba

USER dev

# devcontainer.json mounts a named volume here so environments survive container rebuilds.
# The directory must exist and belong to dev for the volume to inherit that ownership.
ENV MAMBA_ROOT_PREFIX=/home/dev/micromamba
RUN mkdir -p "${MAMBA_ROOT_PREFIX}"
