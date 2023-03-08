import { stringify } from "querystring";
import { isWhiteSpaceLike } from "typescript";

export const enum parseState {
	notParsed,
	startLine,
	headers,
	parsed
}

export class Response {
	statusCode?: number = -1;
	protocol?: string;
	status?: string;
	body?: string;
	statusLine?: string;
	headers: Map<string, string>;
	state: parseState = parseState.notParsed;
	buffer?: string;

	constructor() {
		this.headers = new Map<string, string>();
	}

	// TODO
	private parseBody(body: string): void {
		if ((this.headers.has('Content-Length')
				|| this.headers.has('Transfer-Encoding'))) {
		}
	}

	private parseHeader(field: string): void {
		let [ name, value, ...rest ] = field.split(':');
		if (/[\f\r\t\v ]/.test(name[0]) || /[\f\r\t\v ]/.test(name[name.length - 1])) {
			throw new Error(`Whitespace between start-line and first header field`);
		}
		this.headers?.set(name, value);
	}

	private parseStartline(line: string): void {
		let [ protocol, code, ...status ] = line.split(' ');
		if (!(protocol && code)) {
			throw new Error(`Status line invalid: ${line}`);
		}
		if (protocol !== 'HTTP/1.1') {
			throw new Error(`Protocol invalid: ${protocol}`);
		}
		this.protocol = protocol;
		if (!isNumber(code)) {
			throw new Error(`Status code invalid: ${code}`);
		}
		this.statusCode = Number(code);
		if (status) {
			this.status = status.join(' ');
		}
	}

	parse(data: string): void {
		let index: number = 0;
		// console.log(`\x1b[34m${data}\x1b[0m`);
		while (this.state !== parseState.headers) {
			index = data.indexOf('\r\n');
			if (index === -1) {
				break;
			}
			let str = data.substring(0, index);
			switch (this.state) {
			case parseState.notParsed:
				// console.log(`\x1b[35m${str}\x1b[0m`);
				this.parseStartline(str);
				this.state = parseState.startLine;
				break;
			case parseState.startLine:
				if (!str.length) {
					this.state = parseState.headers;
				} else {
					this.parseHeader(str);
				}
				break;
			default:
				break;
			}
			data = data.substring(index + 2);
			this.buffer = data;
		}
		// console.log(this.buffer);
		if (this.buffer)
			this.parseBody(this.buffer);
		this.state = parseState.parsed;
	}
}

export class Request {
	message: string = '';

	constructor(public requestLine?: string,
				public headers?: string[],
				public body?: string) {
		if (this.requestLine) {
			this.message = this.requestLine;
		}
		if (this.headers) {
			for (let header of this.headers) {
				this.message += header;
				this.message += '\r\n';
			}
		} else {
			this.message += '\r\n';
		}
		this.message += '\r\n';
		if (this.body) {
			this.message += this.body;
		}
	}
}

function isNumber(value: string | number): boolean
{
	return ((value != null) &&
			(value !== '') &&
			!isNaN(Number(value.toString())));
}