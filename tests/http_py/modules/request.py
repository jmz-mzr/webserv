from webtest import *


class Request(TestCase):

    @staticmethod
    def test_method_bad_name() -> str:
        request = f"get / HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_method_unknown() -> str:
        request = f"LOREMIPSUM / HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 501 and response.status != 403 and response.status != 405:
            return f"Status code: {response.getcode()}, expected: 501 | 405 | 403"
        return ""

    @staticmethod
    def test_uri_too_long() -> str:
        request = "GET /" + "a" * URI_MAX_LEN + f" HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 414:
            return f"Status code: {response.getcode()}, expected: {414}"
        return ""

    @staticmethod
    def test_request_line_empty_lines_before() -> str:
        request = f"\n\r\nGET /lines HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            f"Status code: {response.getcode()}, expected: {200}"
        return ""

    @staticmethod
    def test_request_line_invalid1() -> str:
        request = f"GET / HTTP /1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_request_line_invalid2() -> str:
        request = f"GET / HTTP/1.1 error\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_request_line_invalid3() -> str:
        request = f"GET HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_http_version_not_supported() -> str:
        request = f"GET / HTTP/2.0\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 505:
            return f"Status code: {response.getcode()}, expected: {505}"
        return ""

    @staticmethod
    def test_uri_bad1() -> str:
        request = f"GET * HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_uri_bad2() -> str:
        request = f"GET index.php HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_host_missing() -> str:
        request = "GET / HTTP/1.1\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_host_in_header1() -> str:
        request = f"GET / HTTP/1.1\r\nHost: {SERVER_HOST}:{SERVER_PORT}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: {200}"
        return ""

    @staticmethod
    def test_host_in_header2() -> str:
        request = f"GET /index/upload HTTP/1.1\r\nHost: test:1234567\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 301:
            return f"Status code: {response.getcode()}, expected: {301}"
        if response.getheader('Location') != "http://test:8082/index/upload/":
            return (f"Location: {response.getheader('Location')}"
                    + ", expected: \"http://test:8082/index/upload/\"")
        return ""

    @staticmethod
    def test_host_multiple_header() -> str:
        request = f"GET / HTTP/1.1\r\nHost: {SERVER_HOST}\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_host_unknown() -> str:
        request = "GET / HTTP/1.1\r\nHost: unknown\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: {200}"
        return ""

    @staticmethod
    def test_host_invalid1() -> str:
        request = "GET http://unknown@host/ HTTP/1.1\r\nHost: tryAgain\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_host_invalid2() -> str:
        request = "GET http://.:8082/ HTTP/1.1\r\nHost: tryAgain\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_host_invalid3() -> str:
        request = "GET / HTTP/1.1\r\nHost: a..b:8082\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_header_space_before_colon() -> str:
        request = f"GET / HTTP/1.1\r\nHost : {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_header_missing_name() -> str:
        request = f"GET / HTTP/1.1\r\nHost:{SERVER_HOST}\r\n:empty_name\r\n\r\n"
        response = send_request(request)
        if response.status != 400:
            return f"Status code: {response.status}, expected: {400}"
        return ""

    @staticmethod
    def test_header_case_insensitive() -> str:
        request = f"GET / HTTP/1.1\r\nhoSt:{SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.status != 200:
            return f"Status code: {response.status}, expected: {200}"
        return ""

    @staticmethod
    def test_request_line_missing() -> str:
        request = f"Host: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_cr_bare1() -> str:
        request = f"GET / HTTP/1.1\rHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_cr_bare2() -> str:
        request = f"\rGET / HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_tab_space1() -> str:
        request = f"GET /\tHTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_tab_space2() -> str:
        request = f"GET / HTTP/1.1\r\nHost:\t{SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""
