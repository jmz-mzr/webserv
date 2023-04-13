from webtest import *


class Get(TestCase):
    @staticmethod
    def test_valid_path() -> str:
        request = f"GET / HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: 200"
        return ""

    @staticmethod
    def test_almost_done() -> str:
        request = [
            "GET / HTTP/1.1\r",
            f"\nHost: {SERVER_HOST}\r\nTest: get_almost_done\r\n\r\n"
        ]
        response = send_request(*request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: 200"
        return ""

    @staticmethod
    def test_invalid_path() -> str:
        request = f"GET /bad_path/ HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 404:
            return f"Status code: {response.getcode()}, expected: 404"
        return ""

    @staticmethod
    def test_forbidden_path() -> str:
        request = f"GET /post/ HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 405 and response.status != 403:
            return f"Status code: {response.getcode()}, expected: 405 | 403"
        allow_header = response.headers.get('Allow')
        if not allow_header:
            return "\"Allow\" header field missing from response"
        if allow_header != 'POST':
            return f"\"Allow\" header field value: {allow_header}, expected: \"POST\""
        return ""

    @staticmethod
    def test_accept() -> str:
        request = f"GET /index.html HTTP/1.1\r\nHost: {SERVER_HOST}\r\nAccept: image/png\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200 and response.getcode() != 406:
            return f"Status code: {response.getcode()}, expected: 200 | 406"
        return ""

    @staticmethod
    def test_if_modified_since() -> str:
        request = "GET /index.html HTTP/1.1\r\nHost: localhost:8080\r\nIf-Modified-Since: Sat, 26 Mar 2024 12:00:00 " \
                  "GMT\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 304:
            return f"Status code: {response.getcode()}, expected: 304"
        return ""

    @staticmethod
    def test_connection_close() -> str:
        request = "GET /index.html HTTP/1.1\r\nHost: localhost:8080\r\nConnection: close\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: 200"
        connection_header = response.headers.get('Connection')
        if not connection_header:
            return "\"Connection\" header field missing from response"
        if connection_header.lower() != 'close':
            return f"\"Connection\" header field value: {connection_header}, expected: \"close\""
        return ""

    @staticmethod
    def test_content_length() -> str:
        request = "GET /index.html HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: 200"
        content_length_header = response.headers.get('Content-Length')
        if content_length_header and content_length_header != '6':
            return f"\"Content-Length\" header field value: {content_length_header}, expected: \"6\""
        return ""
