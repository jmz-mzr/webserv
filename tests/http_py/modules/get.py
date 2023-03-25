from webtest import *


class GetMethod(TestCase):
	def test_valid_path():
		request = "GET / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
		response = send_request(request)
		if response.getcode() != 200:
			return "Bad status code {}, expect {}".format(response.getcode(), 200)
		return ""

	def test_invalid_path():
		request = "GET /bad_path/ HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
		response = send_request(request)
		if response.getcode() != 404:
			return "Bad status code {}, expect {}".format(response.getcode(), 404)
		return ""