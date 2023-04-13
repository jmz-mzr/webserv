from webtest import *
import socket


class Head(TestCase):
    @staticmethod
    def test_valid_path() -> str:
        request = f"HEAD / HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: 200"
        try:
            body = response.read()
            if body:
                return "HEAD should not return a body"
        except socket.timeout:
            return ""
        return ""
