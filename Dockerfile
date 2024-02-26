FROM debian:bookworm as builder

WORKDIR /app
COPY . .

RUN apt-get -y update && apt-get -y install \
    build-essential \
    cmake \
    wget \
    libboost-filesystem1.81-dev \
    libboost-system1.81-dev \
    libboost-thread1.81-dev \
    libboost-chrono1.81-dev \
    libboost-date-time1.81-dev \
    default-libmysqlclient-dev \
    libgtest-dev

RUN wget https://github.com/BNETDocs/bncsutil/releases/download/v1.4.2/libbncsutil-dev_v1.4.2_amd64.deb && apt-get install ./libbncsutil-dev_v1.4.2_amd64.deb

RUN cmake -G "Unix Makefiles" -B./build && cd build && cmake --build . --config Release && ctest

FROM debian:bookworm-slim

WORKDIR /app

RUN apt-get -y update && apt-get -y install \
    wget \
    libboost-filesystem1.81.0 \
    libboost-system1.81.0 \
    libboost-thread1.81.0 \
    libboost-chrono1.81.0 \
    libboost-date-time1.81.0 \
    default-libmysqlclient-dev

RUN wget https://github.com/BNETDocs/bncsutil/releases/download/v1.4.2/libbncsutil-dev_v1.4.2_amd64.deb \
    && apt-get install ./libbncsutil-dev_v1.4.2_amd64.deb && rm libbncsutil-dev_v1.4.2_amd64.deb

COPY --from=builder /app/build/guard-1.0.2 .
COPY ip-to-country.csv .
COPY guard.cfg .

RUN chmod +x guard-1.0.2

ENTRYPOINT ["/app/guard-1.0.2"]