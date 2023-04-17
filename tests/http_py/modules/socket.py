import socket
from webtest import *

class Socket(TestCase):

	@staticmethod
	def test_broken_pipe() -> str:
		request = f"GET / HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s.connect((SERVER_HOST, SERVER_PORT))
		s.send(request.encode('utf-8'))
		s.close()
		s2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s2.connect((SERVER_HOST, SERVER_PORT))
		response = send_request(request)
		if response.getcode() != 200:
			return f"Status code: {response.getcode()}, expected: 200"
		return ""
