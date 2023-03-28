from webtest import *


class Request(TestCase):

    @staticmethod
    def test_bad_method_name() -> str:
        request = f"get / HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_unknown_method() -> str:
        request = f"LOREMIPSUM / HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 501:
            return f"Status code: {response.getcode()}, expected: {501}"
        return ""

    @staticmethod
    def test_long_uri() -> str:
        request = "GET /" + "a" * URI_MAX_LEN + f" HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 414:
            return f"Status code: {response.getcode()}, expected: {414}"
        return ""

    @staticmethod
    def test_invalid_request_line1() -> str:
        request = f"GET / HTTP /1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_invalid_request_line2() -> str:
        request = f"GET / HTTP/1.1 error\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_invalid_request_line3() -> str:
        request = f"GET HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_version_not_supported() -> str:
        request = f"GET / HTTP/2.0\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 505:
            return f"Status code: {response.getcode()}, expected: {505}"
        return ""

    @staticmethod
    def test_bad_uri1() -> str:
        request = f"GET * HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_bad_uri2() -> str:
        request = f"GET index.php HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_missing_host_header() -> str:
        request = "GET / HTTP/1.1\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_alternative_host_header() -> str:
        request = f"GET / HTTP/1.1\r\nHost: {SERVER_HOST}:{SERVER_PORT}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: {200}"
        return ""

    @staticmethod
    def test_multiple_host_header() -> str:
        request = f"GET / HTTP/1.1\r\nHost: {SERVER_HOST}\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_unknown_host() -> str:
        request = "GET / HTTP/1.1\r\nHost: unknown\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_invalid_host() -> str:
        request = "GET / HTTP/1.1\r\nHost: unknown@host\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_space_before_colon() -> str:
        request = f"GET / HTTP/1.1\r\nHost : {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_missing_header_name() -> str:
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
    def test_missing_request_line() -> str:
        request = f"Host: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_bare_cr1() -> str:
        request = f"GET / HTTP/1.1\rHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Status code: {response.getcode()}, expected: {400}"
        return ""

    @staticmethod
    def test_bare_cr2() -> str:
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
