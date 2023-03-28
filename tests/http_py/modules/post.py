from webtest import *
from standard_tests import *


class Post(TestCase):
    @staticmethod
    def test_post_directory():
        request = "POST / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 409:
            return f"Status code: {response.getcode()}, expected: 409"
        return ""

    @staticmethod
    def test_no_post():
        request = "POST /no_post HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 500:
            return f"Status code: {response.getcode()}, expected: 500"
        return ""

    @staticmethod
    def test_zero_length():
        request = "POST /zero_length HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\n\r\nHello World\r\n\r\n"
        response = send_request(request)
        if not compare_body(response):
            return f"Body does not match Content-Length header : {response.getheader('Content-Length')}"
        if response.getcode() != 201 and response.getcode() != 303:
            return f"Status code: {response.getcode()}, expected: 201|303"
        return ""

    @staticmethod
    def test_no_length():
        request = "POST /no_length HTTP/1.1\r\nHost: localhost\r\n\r\nHello World\r\n\r\n"
        response = send_request(request)
        if not compare_body(response):
            return f"Body does not match Content-Length header : {response.getheader('Content-Length')}"
        if response.getcode() != 500:
            return f"Status code: {response.getcode()}, expected: 500"
        return ""

    @staticmethod
    def test_valid_post():
        request = "POST /valid_post HTTP/1.1\r\nHost: localhost\r\nContent-Length: 11\r\n\r\nHello World\r\n\r\n"
        response = send_request(request)
        if not compare_body(response):
            return f"Body does not match Content-Length header : {response.getheader('Content-Length')}"
        if response.getcode() != 201 and response.getcode() != 409:
            return f"Status code: {response.getcode()}, expected: 201|409"
        return ""
