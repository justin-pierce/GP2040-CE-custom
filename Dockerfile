FROM ubuntu:22.04

WORKDIR /usr/src/app

RUN apt-get install apt-transport-https -y
RUN apt-get update
RUN apt-get install gcc-arm-linux-gnueabi -y
RUN apt-get install cmake -y
RUN apt-get install git -y
RUN apt-get install gcc-arm-none-eabi -y
RUN apt-get install python-is-python3 -y
RUN apt-get install build-essential -y
RUN apt-get install curl -y
RUN curl -sL https://deb.nodesource.com/setup_18.x -o /tmp/nodesource_setup.sh
RUN bash /tmp/nodesource_setup.sh
RUN apt-get install -y nodejs

COPY build-tools/ ./build-tools/

RUN chmod +x build-tools/build.sh

CMD [ "build-tools/build.sh" ]