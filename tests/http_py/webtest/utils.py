import socket
from http.client import HTTPResponse
from .config import *


def send_request(string: str) -> HTTPResponse:
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.connect((SERVER_HOST, SERVER_PORT))
	s.send(string.encode())
	response = HTTPResponse(s)
	response.begin()
	return response
