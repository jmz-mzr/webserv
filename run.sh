#!/bin/bash

docker stop webserv
docker build -t webserv -f=test/Dockerfile .
docker run --rm -d --network host --name webserv_debug webserv:latest