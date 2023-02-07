import * as fs from 'fs'
import * as path from 'path'
import * as net from 'net'

export interface Headers {
	[name: string]: string
}

export class Response {
	protocol: string;
	statusCode: number;
	status?: string;

	constructor(protocol: string, statusCode: number, status?: string) {
		this.statusCode = statusCode;
		this.protocol = protocol;
		if (status) {
			this.status = status;
		}
	}
}

export class Request {
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

const filePath = path.join(__dirname, '../../data/requests.json')
const tests = require(filePath);

for (let test of tests) {

	const socket = net.connect(test.port, test.host);
	const request = new Request(test.requestLine, test.headers, test.body);

	socket.on('connect', () => {
		socket.write(request.buffer);
	});
	socket.on('data', (data) => {
		//TODO: parsing de reponse pour comparer avec data.expectedCode
		console.log(data.toString());
		socket.end();
	});
	socket.on('timeout', () => {
		console.log('socket timeout');
		socket.end();
	});
	socket.on('end', () => {
		console.log('disconnected from server');
	  });

}