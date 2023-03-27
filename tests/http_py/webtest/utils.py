import socket
from http.client import HTTPResponse
from .config import (SERVER_HOST, SERVER_PORT)


def send_request(string: str) -> HTTPResponse:
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.settimeout(5.0)
	s.connect((SERVER_HOST, SERVER_PORT))
	s.send(string.encode())
	response = HTTPResponse(s)
	response.begin()
	return response
