FROM	gcc:latest

WORKDIR	/usr/src/googletest/

RUN		apt-get update \
&&		apt-get install --no-install-recommends cmake \
&&		apt-get clean \
&&		rm -rf /var/lib/apt/lists/*

RUN		git clone https://github.com/google/googletest.git -b release-1.12.1
RUN		mkdir googletest/build

WORKDIR	/usr/src/googletest/build

RUN		cmake .. -DBUILD_GMOCK=OFF\
&&		make \
&&		make install \
&&		rm -rf /usr/src/googletest

WORKDIR	/usr/src/webserv

COPY	. .

RUN 	mkdir 
RUN		make test
RUN		make install

CMD		[ "webserv" ]