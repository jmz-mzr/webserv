import * as path from 'path'
import * as net from 'net'
import * as http from 'http'
import { parseState, Request, Response } from '../http'
import { TestData, ITestSuite } from '../types'



export const TestSuite: ITestSuite = {
	name: "request_parsing",
	init() {
		const workdir = process.env.INIT_CWD?.toString();
		if (!workdir) {
			throw new Error('Cannot find current working directory');
		}
		const filePath: string = path.join(workdir, 'data/request_parsing copy.json');
		if (!filePath) {
			throw new Error('Cannot find test configuration file');
		}
		this.data = require(filePath);
	},
	execute() {
		if (!this.data) {
			throw new Error('Cannot parse configuration file');
		}
		let { suiteFixture, testCases } = this.data;
		let currTest = new TestData();

		for (let param in suiteFixture) {
			currTest[param] = suiteFixture[param];
		}
		
		for (let testCase of testCases)
		{
			for (let param in testCase) {
				currTest[param] = testCase[param];
			}
			const postData = JSON.stringify({
				'msg': 'Hello World!',
			});
			const options = {
				hostname: 'localhost',
				port: 8080,
				path: '/',
				method: 'GET',
				headers: {
				  'Content-Type': 'application/json',
				  'Content-Length': Buffer.byteLength(postData),
				  'Host~': 'oui',
				},
			  };
			const req = http.request(options, (res) => {
				/* Print Headers
					console.log(`HEADERS: ${JSON.stringify(res.headers)}`);*/
				let output: string = `${testCase.name}: `;
				if (testCase.expectedCode === res.statusCode) {
					output += '\x1b[32mSUCCESS\x1b[0m';
				} else {
					output += `\x1b[31mFAIL\x1b[0m Expected: ${testCase.expectedCode} | Actual: \x1b[31m${res.statusCode}\x1b[0m`;
				}
				res.setEncoding('utf8');
				// Need to listen for the data event or else the callback will infinite loop
				res.on('data', (chunk) => {
				//	console.log(`BODY: ${chunk}`);
				});
				/*res.on('end', () => {
				  console.log('No more data in response.');
				});*/
				console.log(output);
			  });
	
			  req.on('error', (e) => {
				console.error(`problem with request: ${e}`);
			  });
			req.write(postData);
			req.end();
		//	console.log(req);
		}
		
	}
}