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

class Fixture {
	[key: string]: any;
	host?: string;
	port?: number;
	requestLine?: string;
	headers?: string[];
	body?: string;
}

class Request extends Fixture {
	message: string = '';

	constructor(fixture: Fixture) {
		super();

		for (let param in fixture) {
			this[param] = fixture[param];
		}
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

const workdir = process.env.INIT_CWD?.toString()
let filePath: string;
if (workdir) {
	filePath = path.join(workdir, 'data/bad_requests.json');
} else {
	throw new Error('Cannot find test configuration file');
}
const testSuite = require(filePath);
const testFixture = new Fixture();
for (let param in testSuite.fixture) {
	testFixture[param] = testSuite.fixture[param];
}


for (let testUnit of testSuite.units) {
	for (let param in testUnit) {
		testFixture[param] = testUnit[param];
	}

	let socket: net.Socket;
	if (testFixture.port) {
		socket = net.connect(testFixture.port, testFixture.host);
	} else {
		throw new Error('No port specified');
	}

	const request = new Request(testFixture);

	socket.on('connect', () => {
		socket.write(request.message);
	});
	socket.on('data', (data) => {
		const response = new Response(data.toString());
		const code = testUnit.expectedCode.find((c: number, i: number) => c == response.statusCode);
		let output = `${testUnit.name}: `;
		if (code) {
			output += `\x1b[32mSUCCESS\x1b[0m`;
		} else {
			output += `\x1b[31mFAIL\x1b[0m Expected: ${testUnit.expectedCode} | Actual: \x1b[31m${response.statusCode}\x1b[0m`;
		}
		console.log(output);
		socket.end();
	});
	socket.on('error', () => {
		console.log(`${testUnit.name}: \x1b[31mERROR\x1b[0m`)
	});
	socket.on('end', () => {
		// console.log('disconnected from server');
	});

}