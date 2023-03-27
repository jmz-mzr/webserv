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
	
	def test_accept():
		request = "GET /index.html HTTP/1.1\r\nHost: localhost:8080\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n\r\n"
		response = send_request(request)
		if response.getcode() != 200:
			return "Bad status code {}, expect {}".format(response.getcode(), 404)
		return ""
	
	def test_if_modified_since():
		request = "GET /index.html HTTP/1.1\r\nHost: localhost:8080\r\nIf-Modified-Since: Sat, 26 Mar 2022 12:00:00 GMT\r\n\r\n"
		response = send_request(request)
		if response.getcode() != 200:
			return "Bad status code {}, expect {}".format(response.getcode(), 404)
		return ""
	
	def test_close():
		request = "GET /index.html HTTP/1.1\r\nHost: localhost:8080\r\nConnection: close\r\n\r\n"
		response = send_request(request)
		if response.getcode() != 200:
			return "Bad status code {}, expect {}".format(response.getcode(), 404)
		connection_header = response.headers.get('Connection')
		if connection_header and connection_header.lower() != 'close':
			return "Error: Connection should be closed"
		return ""