import socket
from http.client import HTTPResponse
from webtest import *

class HeaderFieldMethod(TestCase):
    def test_mandatory_headers() -> str:
        request_header = "GET / HTTP/1.1\r\nHost:{}\r\n\r\n".format(SERVER_HOST)
        http_response = send_request(request_header)
        if http_response.status != 200:
            return "Bad status code: {}, expected: {}".format(
                str(http_response.status), "200"
            )
        headers = http_response.headers
        # print(headers)
        if (
            headers["Content-Length"] == None
            or headers["Date"] == None
            or headers["Server"] == None
            or headers["Last-Modified"] == None
        ):
            return "Missing headers"
        return ""

    def test_case_insensitive() -> str:
        request_header = "GET / HTTP/1.1\r\nhoSt:{}\r\n\r\n".format(SERVER_HOST)
        http_response = send_request(request_header)
        if http_response.status != 200:
            return "Bad status code: {}, expected: {}".format(
                str(http_response.status), "200"
            )
        return ""
    
    def test_space_before_colon() -> str:
        request_header = "GET / HTTP/1.1\r\nHost :{}\r\n\r\n".format(SERVER_HOST)
        http_response = send_request(request_header)
        if http_response.status != 400:
            return "Bad status code: {}, expected: {}".format(
                str(http_response.status), "400"
            )
        request_header = (
            "GET / HTTP/1.1\r\nHost:{}\r\nAccept-Language :hyeyoo\r\n\r\n".format(
                SERVER_HOST
            )
        )
        http_response = send_request(request_header)
        if http_response.status != 400:
            return "Bad status code: {}, expected: {}".format(
                str(http_response.status), "400"
            )
        return ""
    
    def test_header_too_long() -> str:
        long_text = "T" * 100000
        request_header = (
            "GET / HTTP/1.1\r\nHost: {}\r\nUser-Agent: {}\r\n\r\n".format(SERVER_HOST,long_text)
        )
        http_response = send_request(request_header)
        # print(http_response.status)
        if http_response.status // 100 != 4:
            return "Bad status code: {}, expected: {} (NOT MANDATORY)".format(
                str(http_response.status), "4XX"
            )
        return ""
    
    def test_missing_header_name() -> str:
        request_header = "GET / HTTP/1.1\r\nHost:{}\r\n:empty_name\r\n\r\n".format(
            SERVER_HOST
        )
        http_response = send_request(request_header)
        if http_response.status // 100 != 4:
            return "Bad status code: {}, expected: {}".format(
                str(http_response.status), "4XX"
            )
        return ""