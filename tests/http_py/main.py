import socket
from http.client import HTTPResponse
from enum import Enum

HOST = "localhost"
PORT = 8080

class Status(Enum):
    FAIL = False
    SUCCESS = True

def send_request(string: str) -> HTTPResponse :
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    s.send(string.encode())
    response = HTTPResponse(s)
    return response

error_msg = ""

def test_get() -> Status:
    request = "GET / HTTP/1.1\r\nHost: localhost:8080\r\n\r\n"
    response = send_request(request)
    if response.getcode() != 200:
        error_msg = "Bad status code {}, required {}".format(response.getcode(), 200)
        return Status.FAIL
    return Status.SUCCESS

if __name__ == "__main__":
    test_get()