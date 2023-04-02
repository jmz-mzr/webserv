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
        if response.status != 201:
            return f"Status code: {response.status}, expected: 201"
        return ""

    @staticmethod
    def test_trailer_fields() -> str:
        request = [
            "POST /post/trailer HTTP/1.1\r\nHost: localhost\r\nTransfer-encoding: chunked\r\n",
            "5\r\ntest\n\r\n",
            "0\r\naccept-language: fr\r\ntest-header: blabla\r\n\r\n"
        ]
        response = send_request(*request)
        if response.status != 201:
            return f"Status code: {response.status}, expected: 201"
        return ""

    #
    # @staticmethod
    # def test_chunked_multiple_zeros() -> str:
    #     client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #     client.connect((SERVER_HOST, SERVER_PORT))
    #     request_header = "POST /post/print.cgi HTTP/1.1\r\nHost: localhost\r\nTransfer-encoding: chunked\r\n\r\n"
    #     client.send(request_header.encode())
    #     request_header = "5\r\ntest\n\r\n"
    #     client.send(request_header.encode())
    #     request_header = "00000\r\n\r\n"
    #     client.send(request_header.encode())
    #
    #     # read and parse http response
    #     http_response = HTTPResponse(client)
    #     http_response.begin()
    #     if http_response.status != 201 and http_response.status != 303:
    #         return "Status code: {}, expected: {}".format(
    #             str(http_response.status), "201"
    #         )
    #     # print(body)
    #     return ""
    #
    # @staticmethod
    # def test_chunk_extension_no_value() -> str:
    #     client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #     client.connect((SERVER_HOST, SERVER_PORT))
    #     request_header = "POST /post/print.cgi HTTP/1.1\r\nHost: localhost\r\nTransfer-encoding: chunked\r\n\r\n"
    #     client.send(request_header.encode())
    #     request_header = "5\r\ntest\n\r\n000;dude\r\n\r\n"
    #     client.send(request_header.encode())
    #
    #     # read and parse http response
    #     http_response = HTTPResponse(client)
    #     http_response.begin()
    #     if http_response.status != 201 and http_response.status != 303:
    #         return "Status code: {}, expected: {}".format(
    #             str(http_response.status), "201"
    #         )
    #     # print(body)
    #     return ""
    #
    # @staticmethod
    # def test_ChunkExtensionUnquotedValue() -> str:
    #     client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #     client.connect((SERVER_HOST, SERVER_PORT))
    #     request_header = "POST /post/print.cgi HTTP/1.1\r\nHost: localhost\r\nTransfer-encoding: chunked\r\n\r\n"
    #     client.send(request_header.encode())
    #     request_header = "5\r\ntest\n\r\n"
    #     client.send(request_header.encode())
    #     request_header = "000;dude=best\r\n\r\n"
    #     client.send(request_header.encode())
    #
    #     # read and parse http response
    #     http_response = HTTPResponse(client)
    #     http_response.begin()
    #     if http_response.status != 201 and http_response.status != 303:
    #         return "Status code: {}, expected: {}".format(
    #             str(http_response.status), "201"
    #         )
    #     # print(body)
    #     return ""
    #
    # @staticmethod
    # def test_multiple_chunk_extensions() -> str:
    #     client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #     client.connect((SERVER_HOST, SERVER_PORT))
    #     request_header = "POST /post/print.cgi HTTP/1.1\r\nHost: localhost\r\nTransfer-encoding: chunked\r\n\r\n"
    #     client.send(request_header.encode())
    #     request_header = '000;Foo=Bar;dude="The dude is the best";Spam=12345!\r\n\r\n'
    #     client.send(request_header.encode())
    #
    #     # read and parse http response
    #     http_response = HTTPResponse(client)
    #     http_response.begin()
    #     if http_response.status != 201 and http_response.status != 303:
    #         return "Status code: {}, expected: {}".format(
    #             str(http_response.status), "201"
    #         )
    #     # print(body)
    #     return ""
    #
    # @staticmethod
    # def test_decode_empty_body_in_pieces() -> str:
    #     client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #     client.connect((SERVER_HOST, SERVER_PORT))
    #     request_header = "POST /post/print.cgi HTTP/1.1\r\nHost: localhost\r\nTransfer-encoding: chunked\r\n\r\n"
    #     client.send(request_header.encode())
    #     request_header = "0"
    #     client.send(request_header.encode())
    #     request_header = "\r"
    #     client.send(request_header.encode())
    #     request_header = "\n"
    #     client.send(request_header.encode())
    #     request_header = "\r"
    #     client.send(request_header.encode())
    #     request_header = "\n"
    #     client.send(request_header.encode())
    #
    #     # read and parse http response
    #     http_response = HTTPResponse(client)
    #     http_response.begin()
    #     if http_response.status != 201 and http_response.status != 303:
    #         return "Status code: {}, expected: {}".format(
    #             str(http_response.status), "201"
    #         )
    #     # print(body)
    #     return ""
    #
    # @staticmethod
    # def test__decode_three_chunks_one_piece() -> str:
    #     client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #     client.connect((SERVER_HOST, SERVER_PORT))
    #     request_header = "POST /post/print.cgi HTTP/1.1\r\nHost: localhost\r\nTransfer-encoding: chunked\r\n\r\n"
    #     client.send(request_header.encode())
    #     request_header = "F\r\nHello, World!!!\r\n7\r\nIt's me\r\n0\r\n\r\n"
    #     client.send(request_header.encode())
    #
    #     # read and parse http response
    #     http_response = HTTPResponse(client)
    #     http_response.begin()
    #     if http_response.status != 201 and http_response.status != 303:
    #         return "Status code: {}, expected: {}".format(
    #             str(http_response.status), "201"
    #         )
    #     body = http_response.read().decode("UTF-8")
    #     if body.find("Hello, World!!!It's me") == -1:
    #         return "Missing content 'Hello, World!!!It's me' "
    #     return ""
    #
    # @staticmethod
    # def test_decode_bad_trailer() -> str:
    #     client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #     client.connect((SERVER_HOST, SERVER_PORT))
    #     request_header = "POST /post/print.cgi HTTP/1.1\r\nHost: localhost\r\nTransfer-encoding: chunked\r\n\r\n"
    #     client.send(request_header.encode())
    #     request_header = "0\r\nX-Foo Bar\r\n\r\n"
    #     client.send(request_header.encode())
    #
    #     # read and parse http response
    #     http_response = HTTPResponse(client)
    #     http_response.begin()
    #     if http_response.status != 400:
    #         return "Status code: {}, expected: {}".format(
    #             str(http_response.status), "400"
    #         )
    #     return ""