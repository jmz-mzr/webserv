from webtest import *


class Request(TestCase):

    @staticmethod
    def test_bad_method_name():
        request = f"get / HTTP/1.1\r\nHost: {SERVER_HOST}:{SERVER_PORT}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Bad status code {response.getcode()}, expect {400}"
        return ""

    @staticmethod
    def test_unknown_method():
        request = f"LOREMIPSUM / HTTP/1.1\r\nHost: {SERVER_HOST}:{SERVER_PORT}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 501:
            return f"Bad status code {response.getcode()}, expect {501}"
        return ""

    @staticmethod
    def test_long_uri():
        request = "GET /" + "a" * URI_MAX_LEN + f" HTTP/1.1\r\nHost: {SERVER_HOST}:{SERVER_PORT}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 414:
            return f"Bad status code {response.getcode()}, expect {414}"
        return ""

    @staticmethod
    def test_invalid_request_line1():
        request = f"GET / HTTP /1.1\r\nHost: {SERVER_HOST}:{SERVER_PORT}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            f"Bad status code {response.getcode()}, expect {400}"
        return ""

    @staticmethod
    def test_invalid_request_line2():
        request = f"GET / HTTP/1.1 error\r\nHost: {SERVER_HOST}:{SERVER_PORT}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Bad status code {response.getcode()}, expect {400}"
        return ""

    @staticmethod
    def test_invalid_request_line3():
        request = f"GET HTTP/1.1\r\nHost: {SERVER_HOST}:{SERVER_PORT}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Bad status code {response.getcode()}, expect {400}"
        return ""

    @staticmethod
    def test_version_not_supported():
        request = f"GET / HTTP/2.0\r\nHost: {SERVER_HOST}:{SERVER_PORT}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 505:
            return f"Bad status code {response.getcode()}, expect {505}"
        return ""

    @staticmethod
    def test_bad_uri1():
        request = f"GET * HTTP/1.1\r\nHost: {SERVER_HOST}:{SERVER_PORT}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Bad status code {response.getcode()}, expect {400}"
        return ""

    @staticmethod
    def test_bad_uri2():
        request = f"GET index.php HTTP/1.1\r\nHost: {SERVER_HOST}:{SERVER_PORT}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Bad status code {response.getcode()}, expect {400}"
        return ""

    @staticmethod
    def test_missing_host_header():
        request = "GET / HTTP/1.1\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Bad status code {response.getcode()}, expect {400}"
        return ""

    @staticmethod
    def test_multiple_host_header():
        request = f"GET / HTTP/1.1\r\nHost: {SERVER_HOST}:{SERVER_PORT}\r\nHost: {SERVER_HOST}:{SERVER_PORT}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Bad status code {response.getcode()}, expect {400}"
        return ""

    @staticmethod
    def test_unknown_host():
        request = "GET / HTTP/1.1\r\nHost: unknownho$t:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Bad status code {response.getcode()}, expect {400}"
        return ""

    @staticmethod
    def test_invalid_header():
        request = f"GET / HTTP/1.1\r\nHost : {SERVER_HOST}:{SERVER_PORT}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Bad status code {response.getcode()}, expect {400}"
        return ""

    @staticmethod
    def test_missing_request_line():
        request = f"Host: {SERVER_HOST}:{SERVER_PORT}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Bad status code {response.getcode()}, expect {400}"
        return ""

    @staticmethod
    def test_bare_cr1():
        request = f"GET / HTTP/1.1\rHost: {SERVER_HOST}:{SERVER_PORT}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Bad status code {response.getcode()}, expect {400}"
        return ""

    @staticmethod
    def test_bare_cr2():
        request = f"\rGET / HTTP/1.1\r\nHost: {SERVER_HOST}:{SERVER_PORT}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Bad status code {response.getcode()}, expect {400}"
        return ""

    @staticmethod
    def test_tab_space1():
        request = f"GET /\tHTTP/1.1\r\nHost: {SERVER_HOST}:{SERVER_PORT}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Bad status code {response.getcode()}, expect {400}"
        return ""

    @staticmethod
    def test_tab_space2():
        request = f"GET / HTTP/1.1\r\nHost:\t{SERVER_HOST}:{SERVER_PORT}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return f"Bad status code {response.getcode()}, expect {400}"
        return ""
