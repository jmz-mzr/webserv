from webtest import *


class Get(TestCase):
    @staticmethod
    def test_valid_path() -> str:
        request = f"GET / HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: {200}"
        return ""

    @staticmethod
    def test_invalid_path() -> str:
        request = f"GET /bad_path/ HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 404:
            return f"Status code: {response.getcode()}, expected: {404}"
        return ""

    @staticmethod
    def test_accept() -> str:
        request = f"GET /index.html HTTP/1.1\r\nHost: {SERVER_HOST}\r\nAccept: text/html,application/xhtml+xml," \
                  "application/xml;q=0.9,image/webp,*/*;q=0.8\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: {200}"
        return ""

    @staticmethod
    def test_if_modified_since() -> str:
        request = "GET /index.html HTTP/1.1\r\nHost: localhost:8080\r\nIf-Modified-Since: Sat, 26 Mar 2022 12:00:00 " \
                  "GMT\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: {404}"
        return ""

    @staticmethod
    def test_close() -> str:
        request = "GET /index.html HTTP/1.1\r\nHost: localhost:8080\r\nConnection: close\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: {200}"
        connection_header = response.headers.get('Connection')
        if connection_header and connection_header.lower() != 'close':
            return "Error: Connection should be closed"
        return ""
