FROM ubuntu:24.04 AS builder

RUN apt-get update && apt-get install -y \
    build-essential cmake pkg-config \
    libpqxx-dev libsodium-dev libssl-dev \
    git curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release
RUN cmake --build build --target messenger_server -j$(nproc)

FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    libpqxx-dev libsodium23 libssl3 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/messenger_server /usr/local/bin/messenger_server
RUN chmod +x /usr/local/bin/messenger_server

COPY scripts/wait-for-it.sh /usr/local/bin/wait-for-it.sh
RUN chmod +x /usr/local/bin/wait-for-it.sh

EXPOSE 8080
CMD ["wait-for-it.sh", "db:5432", "--", "messenger_server"]
