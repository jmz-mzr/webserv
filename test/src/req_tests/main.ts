import * as fs from 'fs'
import * as path from 'path'
import * as net from 'net'

class Response {
	body?: string;
	protocol: string;
	statusCode: number = -1;
	status?: string;

	constructor(data: string) {
		let httpMessage = data.split("\r\n\r\n");
		if (!httpMessage[0].length) {
			throw new Error(`HTTP response bad format: ${httpMessage}`);
		}
		if (httpMessage.length > 1) {
			this.body = httpMessage[1];
		}
		let head = httpMessage[0].split("\r\n");
		let statusLine = head[0].split(' ');
		if (statusLine.length < 2) {
			throw new Error(`Status line invalid: ${statusLine}`);
		}
		this.protocol = statusLine[0];

		if (!isNumber(statusLine[1])) {
			throw new Error(`Status code invalid: ${statusLine[1]}`);
		}
		this.statusCode = Number(statusLine[1]);
		
		if (statusLine.length > 2) {
			this.status = statusLine.slice(2).join(' ');
		}
	}
}

class Request {
	buffer: string;
	
	constructor(reqLine: string, headers: string, body: string) {
		this.buffer = reqLine;
		if (headers) {
			for (let header of headers) {
				this.buffer += header;
				this.buffer += '\r\n';
			}
		} else {
			this.buffer += '\r\n';
		}
		this.buffer += '\r\n';
		this.buffer += body;
	}
}

function isNumber(value: string | number): boolean
{
	return ((value != null) &&
			(value !== '') &&
			!isNaN(Number(value.toString())));
}

const filePath = path.join(__dirname, '../../data/requests.json')
const tests = require(filePath);

for (let test of tests) {

	const socket = net.connect(test.port, test.host);
	const request = new Request(test.requestLine, test.headers, test.body);

	socket.on('connect', () => {
		socket.write(request.buffer);
	});
	socket.on('data', (data) => {
		const response = new Response(data.toString());
		if (response.statusCode === test.expectedCode) {
			console.log(`\x1b[32mSUCCESS\x1b[0m\tExpected: ${test.expectedCode}`);
		} else {
			console.log(`\x1b[31mFAIL\x1b[0m\tExpected: ${test.expectedCode} | Actual: \x1b[31m${response.statusCode}\x1b[0m`);
		}
		socket.end();
	});
	socket.on('timeout', () => {
		console.log('socket timeout');
		socket.end();
	});
	socket.on('end', () => {
		// console.log('disconnected from server');
	});

}