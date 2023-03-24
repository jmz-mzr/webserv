import socket
from http.client import HTTPResponse
from enum import Enum
import standard_tests as t_headers

HOST = "localhost"
PORT = 8080

class Status(Enum):
    FAIL = False
    SUCCESS = True

def send_request(string: str) -> tuple[HTTPResponse, socket.socket] :
    print("Sending Request: \n" + string);
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    s.send(string.encode())
    response = HTTPResponse(s)
    return (response, s)

error_msg = ""

def test_get() -> Status:
    request = {"requestline": "GET / HTTP/1.1", "Host":  "localhost:8080"}
    response, sock = send_request(t_headers.dict_to_request(request))
    if response.getcode() != 200:
        error_msg = "Bad status code {}, required {}".format(response.getcode(), 200)
        return Status.FAIL
    t_headers.check_redirection(response, sock);
    if (t_headers.test_headers(response) == False):
        error_msg += "Headers do not match"
    return Status.SUCCESS

if __name__ == "__main__":
    test_get()
    print("Result: \n" + error_msg)