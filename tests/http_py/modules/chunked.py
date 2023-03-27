import socket
from http.client import HTTPResponse
# from ..webtest.config import (SERVER_HOST, SERVER_PORT)
from webtest import *


class ChunkedMethod(TestCase):
    def test_basic_chunked() -> str:
        # return ""
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # client.settimeout(5.0)
        client.connect((SERVER_HOST, SERVER_PORT))
        request_header = "POST /chunked HTTP/1.1\r\nHost: localhost\r\nTransfer-encoding: chunked\r\n\r\n"
        client.send(request_header.encode())
        request_header = "5\r\nhello\r\n"
        client.send(request_header.encode())
        request_header = "0\r\n\r\n"
        client.send(request_header.encode())

        # read and parse http response
        http_response = HTTPResponse(client)
        http_response.begin()
        if http_response.status != 201 and http_response.status != 303:
            return "Bad status code: {}, expected: {}".format(
                str(http_response.status), "201"
            )
        body = http_response.read().decode("UTF-8")
        print(body)
        return ""