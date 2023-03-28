import socket
from http.client import HTTPResponse
from webtest import *


class Response(TestCase):
    @staticmethod
    def test_mandatory_headers() -> str:
        request = f"GET / HTTP/1.1\r\nHost:{SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.status != 200:
            return f"Status code: {response.status}, expected: {200}"
        headers = response.headers
        if (
            headers["Content-Length"] is None
                or headers["Date"] is None
                or headers["Server"] is None
                or headers["Last-Modified"] is None
        ):
            return "Missing headers"
        return ""
