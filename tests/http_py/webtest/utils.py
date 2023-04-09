import socket
from http.client import HTTPResponse
from .config import (SERVER_HOST, SERVER_PORT)
from itertools import islice
import time

def send_request(*args) -> HTTPResponse:
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.settimeout(5.0)
	s.connect((SERVER_HOST, SERVER_PORT))
	for string in islice(args, len(args) - 1):
		s.send(string.encode('utf-8'))
	time.sleep(1/10)
	for string in islice(args, len(args) - 1, len(args)):
		s.send(string.encode('utf-8'))
	response = HTTPResponse(s)
	response.begin()
	return response

def	print_headers(response: HTTPResponse) -> HTTPResponse:
	for header, value in response.headers.items():
		print(header + ": " + value + "\n")
	return response
