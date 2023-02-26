FROM		debian:latest

RUN			apt-get update && apt-get install -y \
			bc \
			cmake \
			coreutils \
			curl \
			git \
			g++ \
			nodejs \
&&			rm -rf /var/lib/apt/lists/*

WORKDIR		/usr/src

RUN			git clone https://github.com/google/googletest.git -b release-1.12.1


WORKDIR		/usr/src/googletest/build

RUN			cmake .. -DBUILD_GMOCK=OFF \
&&			make \
&&			make install

WORKDIR		/usr/src/webserv

COPY		./cpp_gtest .

RUN			make test \
&&			make fclean \
&&			make install

ENTRYPOINT [ "webserv" ]