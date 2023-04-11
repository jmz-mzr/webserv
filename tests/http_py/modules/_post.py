from webtest import *


class Post(TestCase):
    @staticmethod
    def test_post_directory():
        request = "POST /post/ HTTP/1.1\r\nHost: localhost:8080\r\nContent-Length: 0\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 201:
            return f"Status code: {response.getcode()}, expected: 201"
        return ""

    @staticmethod
    def test_almost_done():
        request = [
            "POST /post/ HTTP/1.1\r\nHost: localhost:8080\r\n",
            "Content-Length: 0\r\n\r",
            "\n"
        ]
        response = send_request(*request)
        if response.getcode() != 201:
            return f"Status code: {response.getcode()}, expected: 201"
        return ""

    @staticmethod
    def test_no_body():
        request = "POST /post/no_body HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 500:
            return f"Status code: {response.getcode()}, expected: 500"
        return ""

    @staticmethod
    def test_zero_length():
        request = "POST /post/zero_length HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\n\r\nHello World"
        response = send_request(request)
        if not compare_body(response):
            return f"Body does not match Content-Length header : {response.getheader('Content-Length')}"
        if response.getcode() != 201 and response.getcode() != 303:
            return f"Status code: {response.getcode()}, expected: 201 | 303"
        return ""

    @staticmethod
    def test_no_length():
        request = "POST /post/no_length HTTP/1.1\r\nHost: localhost\r\n\r\nHello World"
        response = send_request(request)
        if not compare_body(response):
            return f"Body does not match Content-Length header : {response.getheader('Content-Length')}"
        if response.getcode() != 500:
            return f"Status code: {response.getcode()}, expected: 500"
        return ""

    @staticmethod
    def test_valid():
        request = [
            "POST /post/valid_post HTTP/1.1\r\nHost: localhost\r\n",
            "Content-Length: 11\r\n\r\n",
            "Hello ",
            "Wo",
            "rld"
        ]
        response = send_request(*request)
        if not compare_body(response):
            return f"Body does not match Content-Length header : {response.getheader('Content-Length')}"
        if response.getcode() != 201 and response.getcode() != 409:
            return f"Status code: {response.getcode()}, expected: 201 | 409"
        return ""
