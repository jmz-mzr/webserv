from webtest import *
import os


class Delete(TestCase):
	@staticmethod
	def test_valid() -> str:
		request = [
			"POST /post/file HTTP/1.1\r\nHost: localhost\r\n",
			"Content-Length: 0\r\n\r\n",
			"\r\n\r\n"
		]
		response = send_request(*request)
		if response.getcode() != 201 and response.getcode() != 409:
			return f"POST request not working, cannot try DELETE"
		request = [
			"DELETE /delete/file HTTP/1.1\r\nHost: localhost\r\n\r\n"
		]
		response = send_request(*request)
		if response.getcode() != 200 and response.getcode() != 202 and response.getcode() != 204:
			return f"Status code: {response.getcode()}, expected: 200 | 202 | 204"
		return ""