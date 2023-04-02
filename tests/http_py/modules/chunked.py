import socket
from http.client import HTTPResponse
from webtest import *


#TODO: add some data check after response has been received
class Chunked(TestCase):
    @staticmethod
    def test_basic() -> str:
        request = [
            "POST /post/basic HTTP/1.1\r\nHost: localhost\r\nTransfer-encoding: chunked\r\n\r\n",
            "5\r\nhello\r\n",
            "0\r\n\r\n"
        ]
        response = send_request(*request)
        if response.status != 201 and response.status != 303:
            return f"Status code: {response.status}, expected: 201 | 303"
        return ""

    @staticmethod
    def test_trailer_fields() -> str:
        request = [
            "POST /post/trailer HTTP/1.1\r\nHost: localhost\r\nTransfer-encoding: chunked\r\n",
            "5\r\ntest\n\r\n",
            "0\r\naccept-language: fr\r\ntest-header: blabla\r\n\r\n"
        ]
        response = send_request(*request)
        if response.status != 201 and response.status != 303:
            return f"Status code: {response.status}, expected: 201 | 303"
        return ""

    @staticmethod
    def test_multiple_zeros() -> str:
        request = [
            "POST /post/multzero HTTP/1.1\r\nHost: localhost\r\nTransfer-encoding: chunked\r\n",
            "5\r\ntest\n\r\n",
            "00000\r\n\r\n"
        ]
        response = send_request(*request)
        if response.status != 201 and response.status != 303:
            return f"Status code: {response.status}, expected: 201 | 303"
        return ""

    @staticmethod
    def test_extension_no_value() -> str:
        request = [
            "POST /post/extnoval HTTP/1.1\r\nHost: localhost\r\nTransfer-encoding: chunked\r\n",
            "5\r\ntest\n\r\n000;dude\r\n\r\n"
        ]
        response = send_request(*request)
        if response.status != 201 and response.status != 303:
            return f"Status code: {response.status}, expected: 201 | 303"
        return ""

    @staticmethod
    def test_extension_unquoted_value() -> str:
        request = [
            "POST /post/extunquotval HTTP/1.1\r\nHost: localhost\r\nTransfer-encoding: chunked\r\n",
            "5\r\ntest\n\r\n",
            "000;dude=best\r\n\r\n"
        ]
        response = send_request(*request)
        if response.status != 201 and response.status != 303:
            return f"Status code: {response.status}, expected: 201 | 303"
        return ""

    @staticmethod
    def test_multiple_extensions() -> str:
        request = [
            "POST /post/multext HTTP/1.1\r\nHost: localhost\r\nTransfer-encoding: chunked\r\n",
            '000;Foo=Bar;dude="The dude is the best";Spam=12345!\r\n\r\n'
        ]
        response = send_request(*request)
        if response.status != 201 and response.status != 303:
            return f"Status code: {response.status}, expected: 201 | 303"
        return ""

    @staticmethod
    def test_empty_body_multiple_send() -> str:
        request = [
            "POST /post/emptymultsend HTTP/1.1\r\nHost: localhost\r\nTransfer-encoding: chunked\r\n",
            "0",
            "\r",
            "\n",
            "\r",
            "\n"
        ]
        response = send_request(*request)
        if response.status != 201 and response.status != 303:
            return f"Status code: {response.status}, expected: 201 | 303"
        return ""

    @staticmethod
    def test_one_send() -> str:
        request = [
            "POST /post/basic2 HTTP/1.1\r\nHost: localhost\r\nTransfer-encoding: chunked\r\n",
            "F\r\nHello, World!!!\r\n7\r\nIt's me\r\n0\r\n\r\n"
        ]
        response = send_request(*request)
        if response.status != 201 and response.status != 303:
            return f"Status code: {response.status}, expected: 201 | 303"
        return ""

    @staticmethod
    def test_bad_trailer() -> str:
        request = [
            "POST /post/ HTTP/1.1\r\nHost: localhost\r\nTransfer-encoding: chunked\r\n",
            "0\r\nX-Foo Bar\r\n\r\n"
        ]
        response = send_request(*request)
        if response.status != 400:
            return f"Status code: {response.status}, expected: 400"
        return ""
