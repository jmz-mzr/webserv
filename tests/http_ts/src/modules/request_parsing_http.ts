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
		// for (let testCase of testCases) {
		// 	for (let param in testCase) {
		// 		currTest[param] = testCase[param];
		// 	}
		// 	let options = currTest;

		// 	console.log(currTest);
		// 	let req = http.request(options, (res)=>{
		// 		console.log(`STATUS: ${res.statusCode}`);
		// 		console.log(`HEADERS: ${JSON.stringify(res.headers)}`);
		// 		res.setEncoding('utf8');
		// 		res.on('data', (chunk) => {
		// 		  console.log(`BODY: ${chunk}`);
		// 		});
		// 		res.on('end', () => {
		// 		  console.log('No more data in response.');
		// 		});
		// 	});
		//}
		
		const postData = JSON.stringify({
			'msg': 'Hello World!',
		});

		const options = {
			hostname: 'www.google.com',
			port: 80,
			path: '/upload',
			method: 'POST',
			headers: {
			  'Content-Type': 'application/json',
			  'Content-Length': Buffer.byteLength(postData),
			},
		  };
		  console.log("OUI");
		const req = http.request(options, (res) => {
			console.log(`STATUS: ${res.statusCode}`);
			console.log(`HEADERS: ${JSON.stringify(res.headers)}`);
			res.setEncoding('utf8');
			res.on('data', (chunk) => {
			  console.log(`BODY: ${chunk}`);
			});
			res.on('end', () => {
			  console.log('No more data in response.');
			});
		  });
		  
		  req.on('error', (e) => {
			console.error(`problem with request: ${e.message}`);
		  });
		console.log(req);
	}
}