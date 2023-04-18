from webtest import *


class Config(TestCase):
    @staticmethod
    def test_server_name_http_1_0_empty_host() -> str:
        request = f"GET / HTTP/1.0\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 403 and response.getcode() != 405:
            return f"Status code: {response.getcode()}, expected: 403 | 405"
        return ""

    @staticmethod
    def test_server_name_case_insensitive() -> str:
        request = f"GET / HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 403 and response.getcode() != 405:
            return f"Status code: {response.getcode()}, expected: 403 | 405"
        return ""

    @staticmethod
    def test_server_name_ip() -> str:
        request = f"GET / HTTP/1.1\r\nHost: 127.127.127.127\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 403 and response.getcode() != 405:
            return f"Status code: {response.getcode()}, expected: 403 | 405"
        return ""

    @staticmethod
    def test_error_page() -> str:
        request = f"GET /error_page/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 404:
            return f"Status code: {response.getcode()}, expected: 404"
        body = response.read()
        if body != b'index\n':
            return f"Body: {body}, expected: b'index\\n'"
        return ""

    @staticmethod
    def test_error_page_redirect_once() -> str:
        request = f"GET /redirect_once/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 404:
            return f"Status code: {response.getcode()}, expected: 404"
        body = response.read()
        if body == b'index\n':
            return f"Body: {body}, expected not to find: b'index\\n'"
        return ""

    @staticmethod
    def test_error_page_get_redirect() -> str:
        request = f"DELETE /error_page/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 404:
            return f"Status code: {response.getcode()}, expected: 404"
        body = response.read()
        if body != b'index\n':
            return f"Body: {body}, expected: b'index\\n'"
        return ""

    @staticmethod
    def test_error_page_absolute_redirect() -> str:
        request = f"GET /absolute_redirect/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 302:
            return f"Status code: {response.getcode()}, expected: 302"
        location_header = response.headers.get('Location')
        if not location_header:
            return "\"Location\" header field missing from response"
        if location_header != 'http://tESt/':
            return (f"\"Location\" header field value: \"{location_header}\""
                    + ", expected: \"http://tESt/\"")
        return ""

    @staticmethod
    def test_max_body_size() -> str:
        request = [
            "GET /max_body_size/ HTTP/1.1\r\nHost: test\r\n",
            "Content-Length: 11\r\n\r\n"
        ]
        response = send_request(*request)
        if response.getcode() != 413:
            return f"Status code: {response.getcode()}, expected: 413"
        return ""

    @staticmethod
    def test_max_body_size_chunked() -> str:
        request = [
            "POST /max_body_size/ HTTP/1.1\r\nHost: test\r\n",
            "Transfer-Encoding: chunked\r\n\r\n",
            "9\r\n123456789\r\n2\r\n",
            "0\r\n\r\n"
        ]
        response = send_request(*request)
        if response.getcode() != 413:
            return f"Status code: {response.getcode()}, expected: 413"
        return ""

    @staticmethod
    def test_limit_except() -> str:
        request = f"DELETE /index/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 403 and response.getcode() != 405:
            return f"Status code: {response.getcode()}, expected: 403 | 405"
        return ""

    @staticmethod
    def test_ignore_except() -> str:
        request = f"HEAD /index/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 404:
            return f"Status code: {response.getcode()}, expected: 404"
        return ""

    @staticmethod
    def test_return_absolute_uri_without_code() -> str:
        request = f"GET /redir/ HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 302:
            return f"Status code: {response.getcode()}, expected: 302"
        if not response.getheader('Location'):
            return f"Location header missing"
        else:
            url = response.getheader('Location')
            if url != "http://localhost:8080/":
                return f'Location header value: {url}, expected: "http://localhost:8080/"'
        request = f"GET {url} HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: 200"
        body = response.read()
        if body != b'index\n':
            return f"Body: {body}, expected: b'index\\n'"
        return ""

    @staticmethod
    def test_return_local_uri_with_redir_code() -> str:
        request = f"POST /return/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 308:
            return f"Status code: {response.getcode()}, expected: 308"
        if not response.getheader('Location'):
            return f"Location header missing"
        else:
            url = response.getheader('Location')
            if url != "http://test:8082/index/":
                return f'Location header value: {url}, expected: "http://test:8082/index/"'
        request = f"GET {url} HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: 200"
        body = response.read()
        if body != b'index\n':
            return f"Body: {body}, expected: b'index\\n'"
        return ""

    @staticmethod
    def test_return_local_uri_with_error_code() -> str:
        request = f"POST /return_error/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 818:
            return f"Status code: {response.getcode()}, expected: 818"
        if response.getheader('Location'):
            return f"Location header should not be in the response"
        body = response.read()
        if body != b'/index/':
            return f"Body: {body}, expected: b'/index/'"
        return ""

    @staticmethod
    def test_return_local_uri_with_body_drop() -> str:
        request = [
            f"GET /return_body_drop/ HTTP/1.1\r\nHost: test\r\n",
            "Content-Length: 4\r\n\r\n",
            "abc\n"
        ]
        response = send_request(*request)
        if response.getcode() != 118:
            return f"Status code: {response.getcode()}, expected: 118"
        if response.getheader('Location'):
            return f"Location header should not be in the response"
        if (response.getheader('Content-Length') and
                response.getheader('Content-Length') != "0"):
            return (f"Content-Length: {response.getheader('Content-Length')}"
                    + ", expected: absent, or \"0\"")
        return ""

    @staticmethod
    def test_return_code() -> str:
        request = [
            f"PUT /return_code/ HTTP/1.1\r\nHost: test\r\n",
            "Content-Length: 4\r\n\r\n",
            "abc\n"
        ]
        response = send_request(*request)
        if response.getcode() != 742:
            return f"Status code: {response.getcode()}, expected: 742"
        if response.getheader('Location'):
            return f"Location header should not be in the response"
        if (response.getheader('Content-Length') and
                response.getheader('Content-Length') != "0"):
            return (f"Content-Length: {response.getheader('Content-Length')}"
                    + ", expected: absent, or \"0\"")
        return ""

    @staticmethod
    def test_error_page_return() -> str:
        request = f"GET /error_page_return/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 818:
            return f"Status code: {response.getcode()}, expected: 818"
        if response.getheader('Location'):
            return f"Location header should not be in the response"
        body = response.read()
        if body != b'/index/':
            return f"Body: {body}, expected: b'/index/'"
        return ""

    @staticmethod
    def test_root_at_root() -> str:
        request = f"GET /index.html HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 404:
            return f"Status code: {response.getcode()}, expected: 404"
        return ""

    @staticmethod
    def test_alias_at_root() -> str:
        request = f"GET /index.html.abc HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 404:
            return f"Status code: {response.getcode()}, expected: 404"
        return ""

    @staticmethod
    def test_root_redefined() -> str:
        request = f"GET /webserv.test/index.html HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: 200"
        body = response.read()
        if body != b'index\n':
            return f"Body: {body}, expected: b'index\\n'"
        return ""

    @staticmethod
    def test_alias_redefined() -> str:
        request = f"GET /webserv.test/index.php HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 301:
            return f"Status code: {response.getcode()}, expected: 301"
        if response.getheader('Location') != "http://test:8082/webserv.test/index.php/":
            return (f"Location: {response.getheader('Location')}"
                    + ", expected: \"http://test:8082/webserv.test/index.php/\"")
        return ""

    @staticmethod
    def test_index_inherited() -> str:
        request = f"GET /index/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: 200"
        body = response.read()
        if body != b'index\n':
            return f"Body: {body}, expected: b'index\\n'"
        return ""

    @staticmethod
    def test_index_redefined() -> str:
        request = f"GET /index/php/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: 200"
        body = response.read(25)
        if body.find(b'php-cgi') == -1:
            return f"Body(25): {body}, expected to find: b'php-cgi'"
        return ""

    @staticmethod
    def test_autoindex_on() -> str:
        request = f"GET /autoindex/ HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: 200"
        body = response.read()
        if body.find(b'<a href="autoindex/">autoindex/</a>') == -1:
            return (f"Body: {body}, expected to find: "
                    + '<a href="autoindex/">autoindex/</a>')
        return ""

    @staticmethod
    def test_autoindex_off() -> str:
        request = f"GET /autoindex/switch.off/ HTTP/1.1\r\nHost: {SERVER_HOST}\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 403:
            return f"Status code: {response.getcode()}, expected: 403"
        return ""

    @staticmethod
    def test_hide_limit_rule_on() -> str:
        request = f"POST /hide_rules/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 405:
            return f"Status code: {response.getcode()}, expected: 405"
        allow_header = response.headers.get('Allow')
        if not allow_header:
            return "\"Allow\" header field missing from response"
        if allow_header != 'GET':
            return f"\"Allow\" header field value: {allow_header}, expected: \"GET\""
        return ""

    @staticmethod
    def test_hide_limit_rule_off() -> str:
        request = f"GET /hide_rules/limit.off/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 403:
            return f"Status code: {response.getcode()}, expected: 403"
        return ""

    @staticmethod
    def test_hide_directory_on() -> str:
        request = f"GET /hide_rules/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 404:
            return f"Status code: {response.getcode()}, expected: 404"
        return ""

    @staticmethod
    def test_hide_directory_off() -> str:
        request = f"GET /hide_rules/dir.off/ HTTP/1.1\r\nHost: test\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 403:
            return f"Status code: {response.getcode()}, expected: 403"
        return ""

    @staticmethod
    def test_location_without_match() -> str:
        request = f"GET / HTTP/1.1\r\nHost: empty\r\n\r\n"
        response = send_request(request)
        if response.getcode() != 200:
            return f"Status code: {response.getcode()}, expected: 200"
        body = response.read()
        if body != b'index\n':
            return f"Body: {body}, expected: b'index\\n'"
        return ""
