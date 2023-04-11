from webtest import *


class Put(TestCase):
    @staticmethod
    def test_valid_create_modify() -> str:
        request = [
            "PUT /put/valid_put HTTP/1.1\r\n",
            "Host: localhost\r\n",
            "Content-Length: 0\r\n\r\n"
        ]
        response = send_request(*request)
        if response.getcode() != 201:
            return f"Status code: {response.getcode()}, expected: 201"
        request = [
            "PUT /put/valid_put HTTP/1.1\r\n",
            "Host: localhost\r\n",
            "Content-Length: 1\r\n\r\n"
            "a"
        ]
        response = send_request(*request)
        if response.getcode() != 200 and response.getcode() != 204:
            return f"Status code: {response.getcode()}, expected: 200 | 204"
        return ""

    @staticmethod
    def test_invalid_media_type() -> str:
        request = [
            "PUT /put/invalid_put HTTP/1.1\r\n",
            "Host: localhost\r\n",
            "Content-Length: 0\r\n"
            "Content-Type: type/inconnu\r\n\r\n"
        ]
        response = send_request(*request)
        if response.getcode() != 415:
            return f"Status code: {response.getcode()}, expected: 415"