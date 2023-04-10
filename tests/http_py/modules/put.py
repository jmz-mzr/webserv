from webtest import *


class Put(TestCase):
    @staticmethod
    def test_valid() -> str:
        request = [
            "PUT /put/valid_put HTTP/1.1\r\n",
            "Host: localhost\r\n",
            "Content-Length: 0\r\n\r\n"
        ]
        response = send_request(*request)
        if not compare_body(response):
            return f"Body does not match Content-Length header : {response.getheader('Content-Length')}"
        if response.getcode() != 200 and response.getcode() != 201 and response.getcode() != 204:
            return f"Status code: {response.getcode()}, expected: 200 | 201 | 204"
        return ""
