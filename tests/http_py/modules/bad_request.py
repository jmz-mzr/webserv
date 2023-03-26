from webtest import *


class BadMethod(TestCase):
    # def test_not_allowed_method():
    #     request = "DELETE / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
    #     response = send_request(request)
    #     if response.getcode() != 405:
    #         return "Bad status code {}, expect {}".format(response.getcode(), 405)
    #     return ""

    def test_wrong_method():
        request = "Get / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""
    
    def test_no_host():
        request = "Get / HTTP/1.1\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""
    
    def test_bad_header():
        request = "Get / HTTP/1.1\r\nHost: localhost:8080\r\nTest~~: oui\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""
    
    def test_bad_version():
        request = "Get / HTTP/144.1\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 400:
            return "Bad status code {}, expect {}".format(response.getcode(), 400)
        return ""