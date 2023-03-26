from http.client import HTTPResponse
import socket

def dict_to_request(d: dict) -> str:
    res = ""
    for header, value in d.items():
        if header == "requestline":
            res += value + "\r\n"
            continue ;
        res += header + ": " + value + "\r\n"
    res += "\r\n"
    return res

def compare_headers(response: HTTPResponse, headers) -> bool :
    return False

def compare_body(response: HTTPResponse, body) -> bool :
    #check if body length match with content-length headers
    if response.read().__len__ != int(response.getheader("Content-Length"))
        return False
    return True

def compare_time(response: HTTPResponse, executionTime) -> bool :
    return False 

def get_headers_by_status_code(code) -> dict :
    headers = {"405": {"Allow": "GET, POST, DELETE"}
               } #MUST
    return headers
    
def check_redirection(response: HTTPResponse, s: socket.socket, requestHeaders: dict) -> bool :
    if response.getcode() == 301:
        requestHeaders.Host = response.getheader("Location")
        s.send(dict_to_request(requestHeaders).encode())

    return False
