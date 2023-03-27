from webtest import *
from standard_tests import *

class PostMethod(TestCase):
    def test_post_directory():
        request = "POST / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 409:
            return "Bad status code {}, expect {}".format(response.getcode(), 200)
        return ""
		
    def test_no_post():
        request = "POST /testing.html HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return "Bad status code {}, expect {}".format(response.getcode(), 200)
        return ""
    
    def test_zero_length():
        request = "POST /example HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\n\r\nHello World\r\n\r\n"
        response = send_request(request)
        if compare_body(response) == False:
            return "Body does not match Content-Length header : {}".format(response.getheader("Content-Length"))
        if response.getcode() != 200:
            return "Bad status code {}, expect {}".format(response.getcode(), 200)
        return ""
    
    def test_no_length():
        request = "POST /example HTTP/1.1\r\nHost: localhost\r\n\r\nHello World\r\n\r\n"
        response = send_request(request)
        if compare_body(response) == False:
            return "Body does not match Content-Length header : {}".format(response.getheader("Content-Length"))
        if response.getcode() != 200:
            return "Bad status code {}, expect {}".format(response.getcode(), 200)
        return ""
    
    def test_valid_post():
        request = "POST /example HTTP/1.1\r\nHost: localhost\r\nContent-Length: 10\r\n\r\nHello World\r\n\r\n"
        response = send_request(request)
        if compare_body(response) == False:
            return "Body does not match Content-Length header : {}".format(response.getheader("Content-Length"))
        if response.getcode() != 200:
            return "Bad status code {}, expect {}".format(response.getcode(), 200)
        return ""