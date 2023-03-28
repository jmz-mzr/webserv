from webtest import *

class HostMethod(TestCase):
    def test_missing_host() -> str:
        request_header = "GET / HTTP/1.1\r\n\r\n"
        http_response = send_request(request_header)
        if http_response.status != 400:
            return "Bad status code: {}, expected: {}".format(
                str(http_response.status), "400"
            )
        return ""


    def test_double_host() -> str:
        request_header = "GET / HTTP/1.1\r\nHost: naver.com\r\nHost: hyeyoo.com\r\n\r\n"
        http_response = send_request(request_header)
        if http_response.status != 400:
            return "Bad status code: {}, expected: {}".format(
                str(http_response.status), "400"
            )
        return ""


    def test_multiple_host() -> str:
        request_header = "GET / HTTP/1.1\r\nHost: {}\r\nHost: {}\r\n\r\n".format(
            SERVER_HOST, SERVER_HOST
        )
        http_response = send_request(request_header)
        if http_response.status != 400:
            return "Bad status code: {}, expected: {}".format(
                str(http_response.status), "400"
            )
        return ""


    def test_invalid_host() -> str:
        request_header = "GET / HTTP/1.1\r\nHost: hyeyoo@hyeyoo.com\r\n\r\n"
        http_response = send_request(request_header)
        if http_response.status != 400:
            return "Bad status code: {}, expected: {}".format(
                str(http_response.status), "400"
            )
        return ""