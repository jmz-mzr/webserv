from webtest import *


class InvalidRequest(TestCase):

    @staticmethod
    def test_bad_method_name():
        request = "get / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""

    @staticmethod
    def test_unknown_method():
        request = "LOREMIPSUM / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 501:
            return "Bad status code {}, expect {}".format(response.getcode(), 501)
        return ""

    @staticmethod
    def test_long_uri():
        request = "GET /" + "a" * URI_MAX_LENGTH + " HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 414:
            return "Bad status code {}, expect {}".format(response.getcode(), 414)
        return ""

    @staticmethod
    def test_invalid_request_line1():
        request = "GET / HTTP /1.1\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""

    @staticmethod
    def test_invalid_request_line2():
        request = "GET / HTTP/1.1 error\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""

    @staticmethod
    def test_invalid_request_line3():
        request = "GET HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""

    @staticmethod
    def test_version_not_supported():
        request = "GET / HTTP/2.0\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 505:
            return "Bad status code {}, expect {}".format(response.getcode(), 505)
        return ""

    @staticmethod
    def test_bad_uri1():
        request = "GET * HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""

    @staticmethod
    def test_bad_uri2():
        request = "GET index.php HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""

    @staticmethod
    def test_missing_host_header():
        request = "GET / HTTP/1.1\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""

    @staticmethod
    def test_multiple_host_header():
        request = "GET / HTTP/1.1\r\nHost: localhost:8080\r\nHost: localhost:8081\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""

    @staticmethod
    def test_unknown_host():
        request = "GET / HTTP/1.1\r\nHost: unknownho$t:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""

    @staticmethod
    def test_invalid_header():
        request = "GET / HTTP/1.1\r\nHost : localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""

    @staticmethod
    def test_missing_request_line():
        request = "Host: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""

    @staticmethod
    def test_bare_cr1():
        request = "GET / HTTP/1.1\rHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""

    @staticmethod
    def test_bare_cr2():
        request = "\rGET / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""

    @staticmethod
    def test_tab_space1():
        request = "GET /\tHTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""

    @staticmethod
    def test_tab_space2():
        request = "GET / HTTP/1.1\r\nHost:\tlocalhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""
