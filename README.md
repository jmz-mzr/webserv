# Webserv

<p align="center">
  <img src="https://github.com/jmz-mzr/readme_tests/assets/60391881/a1f5e47c-8d24-498f-9647-29b422394f37">
</p>

<details align="center">
<summary><b>Table of Contents</b></summary>
<div align="left">
<br>

- [Overview](#overview)
  - [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
  - [How requests are processed](#how-requests-are-processed)
- [Configuration](#configuration)
  - [All the directives](#directives)
- [Debugging and Logs](#debugging-and-logs)
- [Testing](#testing)
- [Uninstallation](#uninstallation)
- [Documentation](#documentation)

</div>
</details>

<br>

## Overview

Webserv is a high-performance, non-blocking web server designed to replicate NGINX's functionality.  
Developed in C++ for MacOS and Linux, it serves static websites, supports large file uploads and downloads, and is configurable to a high degree, just like NGINX.  
It implements the HTTP protocol standards as defined in [RFC 9112](https://www.rfc-editor.org/rfc/rfc9112), [RFC 9110](https://www.rfc-editor.org/rfc/rfc9110), and [RFC 3986](https://www.rfc-editor.org/rfc/rfc3986).

### Features

- Multiple virtual servers IPv4 & IPv6 compatibles
- Customizable route serving, error pages, redirections, response codes
- Default index files for directory requests
- Limit on allowed request methods & client body size, directory listing on/off toggling
- CGI support for external program execution ([RFC 3875](https://www.rfc-editor.org/rfc/rfc3875))
- PHP and Perl interactive website examples with cookie and session management
- Multiple testers allowing a perfect CI flow

<br>

## Installation

```sh
source configure && make install
```

You can change the installation directory with the `INSTALLDIR` variable in the [`configure`](https://github.com/jmz-mzr/webserv/blob/main/configure) file. By default, it is installed in `$HOME/.local`.

<br>

## Usage

```sh
webserv [configuration file]
```

Provide your own configuration file as first argument. Without it, `$INSTALLDIR/etc/webserv/default.conf` is used as the default one.  
The file [`default.conf`](https://github.com/jmz-mzr/webserv/blob/main/default.conf) of this repository is used to run all the provided tests and examples.

The server root is the `$INSTALLDIR/var/www/webserv` directory. After installation, it is a copy of the `www` directory of this repository.  
As with NGINX, use the [root](#configuration) or [alias](#configuration) directives in the virtual servers of your configuration file, to set a custom relative or absolute path for the files served in response to a request.

### How requests are processed

The requested are processed [the same way as NGINX](https://nginx.org/en/docs/http/request_processing.html):

1. Virtual servers listen to an `address[:port]` pair. If no port is given, they listen on port `8000`. Without a given [listen](#configuration) directive, they listen to `*:8000`.
2. When a request reaches the web server through an `address[:port]` pair, the [server_name](#configuration) directive selects which corresponding virtual server will serve it. If the request’s `Host` header field does not match any virtual server name, the first virtual server with a matching `address[:port]` is used.
3. The selected virtual server searches for the most specific prefix location regardless of the listed order. Then it checks nested extension locations in the order listed. The first matching expression is the location used. If no expression matches the request, then the most specific prefix location found earlier is used.

<br>

## Configuration

The configuration file's structure is the same as the [NGINX structure](https://nginx.org/en/docs/beginners_guide.html#conf_structure), with only the `server{}` and `location{}` blocks.  
<span id="directives"/>

The directives are inherited: a child context inherits the settings of its parent level. For example, a `server{}` block directive is inherited by all the `location{}` blocks in it. However, when the same directive is defined again in a child context, it overrides the parent value.

<details>
<summary><b>Here are all the directives</b></summary>
<br>
  
| Directive<br>(links to NGINX version) | Syntax | Default | Informations |
| :---: | :---: |  :---: |  --- |
| <span id="listen">[listen][1]</span> | `listen `*`address[:port]`*`;` | `listen *:8000;` | An address may be a hostname, a IPv4, or a IPv6 in square brackets (e.g.`[::1]`). |
| <span id="server_name">[server_name][2]</span> | `server_name `*`name ...`*`;` | `server_name "";` | Names are case-insensitive. An IP is also a valid name. |
| <span id="root">[root][3]</span> | `root `*`path`*`;` | `root html;` | Sets the root directory for requests. |
| <span id="alias">[alias][4]</span> | `alias `*`path`*`;` | / | Defines a replacement for the specified location. |
| <span id="index">[index][5]</span> | `index `*`file`*`;` | `index index.html;` | Defines a file used as an index. |
| <span id="return">[return][6]</span> | `return `*`code [text]`*`;` / `return `*`code URL`*`;` / `return `*`URL`*`;` | / | Specifies a redirect URL (for codes 301, 302, 303, 307, and 308) or the response body text (for other codes). A redirect URL can be a URI local to this server. |
| <span id="error_page">[error_page][7]</span> | `error_page `*`code ... uri`*`;` | / | Defines the URI that will be shown for the specified error(s). |
| <span id="cgi_pass">cgi_pass</span> | `cgi_pass `*`path`*`;` | / | Defines a script to execute for requests at this specific location. |
| <span id="client_max_body_size">[client_max_body_size][8]</span> | `client_max_body_size `*`size`*`;` | / | Sets the maximum allowed size of the client request body. |
| <span id="ignore_except">ignore_except</span> | `ignore_except `*`method ...`*`;` | / | Ignores the location match for requests with the given method(s). |
| <span id="limit_except">limit_except</span> | `limit_except `*`method ...`*`;` | / | Returns 403 for requests with the given method(s). |
| <span id="hide_limit_rule">hide_limit_rule</span> | `hide_limit_rule `*`on / off`*`;` | `hide_limit_rule off;` | Returns 405 instead of 403 for requests hitting `limit_except`. |
| <span id="autoindex">[autoindex][9]</span> | `autoindex `*`on / off`*`;` | `autoindex off;` | Enables or disables the directory listing output. |
| <span id="hide_directory">hide_directory</span> | `hide_directory `*`on / off`*`;` | `hide_directory off;` | Returns 404 instead of 403 for requests hitting a directory without `autoindex`. |

[1]: https://nginx.org/en/docs/http/ngx_http_core_module.html#listen
[2]: https://nginx.org/en/docs/http/ngx_http_core_module.html#server_name
[3]: https://nginx.org/en/docs/http/ngx_http_core_module.html#root
[4]: https://nginx.org/en/docs/http/ngx_http_core_module.html#alias
[5]: https://nginx.org/en/docs/http/ngx_http_index_module.html#index
[6]: https://nginx.org/en/docs/http/ngx_http_rewrite_module.html#return
[7]: https://nginx.org/en/docs/http/ngx_http_core_module.html#error_page
[8]: https://nginx.org/en/docs/http/ngx_http_core_module.html#client_max_body_size
[9]: https://nginx.org/en/docs/http/ngx_http_autoindex_module.html#autoindex

</details>

<br>

## Debugging and Logs

```sh
make debug
```

The compiled debug version is the `webserv_debug` binary at the root of this repository. It has far more verbose and helpful logs.

Logs are stored in `$INSTALLDIR/var/log/webserv.log`, and in `/tmp/webserv.log` for the debug version.  
If their size exceeds 20MB, they rotate in a new file, appending “.old“ to the previous one.

<br>

## Testing

Three testing suites are provided:

- An extensive [Python tester](https://github.com/jmz-mzr/webserv/tree/main/tests/http_py) for HTTP protocol and configuration directives compliance
- A 42 tester for load stress-testing and large file exchanges with multiple clients
- A [GoogleTest suite](https://github.com/jmz-mzr/webserv/tree/main/tests/cpp_gtest/data) for configuration parsing and possible directives' values

Run the tests from the root of this repository with:

```sh
./tests/run_tests.sh
```

To enable the GoogleTest suite, you need to have CMake installed, then run:

```sh
make test
```

> [!IMPORTANT]
> To run correctly, the testers expect the `default.conf` file of this repository, and `$INSTALLDIR/var/www/webserv` to have at least the content of the `www` directory of this repository.

<div align="center">
  <video src="https://github.com/jmz-mzr/readme_tests/assets/60391881/adc26c19-6469-453c-9a1a-b40087c63a11" height="1080" width="1920" />
</div>

<br>

## Uninstallation

> [!TIP]
>
> ```sh
> source configure -u
> ```
>
> This unsets the webserv-specific environment variables, cleans the `PATH` variable if relevant, removes the installed binary, and all the webserv-related files (after asking for confirmation).

> [!CAUTION]
>
> ```sh
> make uninstall
> ```
>
> This removes the installed binary, the `$INSTALLDIR/etc/webserv` configuration directory, and the `$INSTALLDIR/var/www/webserv` directory serving your files (without asking for confirmation).

<br>

## Documentation

For more details on the HTTP protocol:

- [HTTP Overview](https://developer.mozilla.org/en-US/docs/Web/HTTP)
- [HTTP Methods](https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods)
- [HTTP Headers](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers)
- [HTTP Response Codes](https://developer.mozilla.org/en-US/docs/Web/HTTP/Status)
