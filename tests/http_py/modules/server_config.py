from webtest import *


class Config(TestCase):
    @staticmethod
    def test_http_1_0_empty_host_server_name() -> str:
        request = f"GET / HTTP/1.0\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 403 and response.getcode() != 405:
            return f"Status code: {response.getcode()}, expected: 403 | 405"
        return ""

    @staticmethod
    def test_case_insensitive_server_name() -> str:
        request = f"GET / HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 403 and response.getcode() != 405:
            return f"Status code: {response.getcode()}, expected: 403 | 405"
        return ""

    @staticmethod
    def test_ip_server_name() -> str:
        request = f"GET / HTTP/1.1\r\nHost: 127.127.127.127\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 403 and response.getcode() != 405:
            return f"Status code: {response.getcode()}, expected: 403 | 405"
        return ""

    @staticmethod
    def test_error_page() -> str:
        request = f"GET /error_page/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 404:
            return f"Status code: {response.getcode()}, expected: 404"
        body = response.read();
        if body != b'index\n':
            return f"Body: {body}, expected: b'index\\n'"
        return ""

    @staticmethod
    def test_redirect_once() -> str:
        request = f"GET /redirect_once/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 404:
            return f"Status code: {response.getcode()}, expected: 404"
        body = response.read();
        if body == b'index\n':
            return f"Body: {body}, expected not to find: b'index\\n'"
        return ""

    @staticmethod
    def test_error_page_get_redirect() -> str:
        request = f"DELETE /error_page/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 404:
            return f"Status code: {response.getcode()}, expected: 404"
        body = response.read();
        if body != b'index\n':
            return f"Body: {body}, expected: b'index\\n'"
        return ""

    @staticmethod
    def test_error_page_absolute_redirect() -> str:
        request = f"GET /absolute_redirect/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 302:
            return f"Status code: {response.getcode()}, expected: 302"
        location_header = response.headers.get('Location')
        if not location_header:
            return "\"Location\" header field missing from response"
        if location_header != 'http://tESt/':
            return (f"\"Location\" header field value: \"{location_header}\""
                        + ", expected: \"http://tESt/\"")
        return ""

    @staticmethod
    def test_max_body_size() -> str:
        request = [
            "GET /max_body_size/ HTTP/1.1\r\nHost: test\r\n",
            "Content-length: 11\r\n\r\n"
        ]
        response = send_request(*request)
        if response.getcode() != 413:
            return f"Status code: {response.getcode()}, expected: 413"
        return ""

    @staticmethod
    def test_max_body_size_chunked() -> str:
        request = [
            "POST /max_body_size/ HTTP/1.1\r\nHost: test\r\n",
            "Transfer-Encoding: chunked\r\n\r\n",
            "9\r\n123456789\r\n2\r\n",
            "0\r\n\r\n"
        ]
        response = send_request(*request)
        if response.getcode() != 413:
            return f"Status code: {response.getcode()}, expected: 413"
        return ""
