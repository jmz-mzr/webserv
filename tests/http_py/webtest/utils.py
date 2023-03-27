import socket
from http.client import HTTPResponse
from .config import (SERVER_HOST, SERVER_PORT)


def send_request(string: str, chunked = False) -> HTTPResponse:
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.settimeout(5.0)
	s.connect((SERVER_HOST, SERVER_PORT))
	s.send(string.encode())
	if (chunked == False):
		response = HTTPResponse(s)
	else:
		return  HTTPResponse(status=404)
	response.begin()
	return response
