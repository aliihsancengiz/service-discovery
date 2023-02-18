FROM ubuntu:22.04

RUN apt-get -y update && apt-get install -y

RUN apt-get -y install clang cmake

RUN apt-get -y install libgmock-dev libgtest-dev libboost-all-dev

COPY . /usr/src/Service-Discovery

WORKDIR /usr/src/Service-Discovery

RUN ./build.sh

CMD ["./build/examples/service-announcement"]