FROM postgres:16-bookworm

RUN apt-get update -qq && \
    apt-get install -y -qq \
    postgresql-server-dev-16 \
    build-essential \
    make \
    gcc \
    git \
    valgrind \
    curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

CMD ["make", "clean", "all"]
