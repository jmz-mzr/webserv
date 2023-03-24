import * as path from 'path'
import * as net from 'net'
import * as http from 'http'
import { parseState, Request, Response } from '../http'
import { TestData, ITestSuite } from '../types'
import assert from 'assert'

// Example of a request configuration
// const options = {
// 	hostname: 'localhost',
// 	port: 8080,
// 	path: '/',
// 	method: 'GET',
// 	headers: {
// 	  'Content-Type': 'application/json',
// 	  'Content-Length': Buffer.byteLength(postData),
// 	},
//   };
export const TestSuite: ITestSuite = {
	name: "request_parsing",
	init() {
		const workdir = process.env.INIT_CWD?.toString();
		if (!workdir) {
			throw new Error('Cannot find current working directory');
		}
		const filePath: string = path.join(workdir, 'data/request_parsing_debug.json');
		if (!filePath) {
			throw new Error('Cannot find test configuration file');
		}
		this.data = require(filePath);
	},
	execute() {
		const	compareStatusCode = (data1:any, data2: any, res1:any, res2: any, output: any) =>
		{
			output += "Status code test : ";
			if (res2.statusCode === res1.statusCode) {
				output += '\x1b[32mSUCCESS\x1b[0m \n';
			} else {
				output += `\x1b[31mFAIL\n\x1b[0mExpected: ${res2.statusCode} | Actual: \x1b[31m${res1.statusCode}\x1b[0m \n`;
			}
			console.log(output);
		}
		const	compareHeaders = (data1:any, data2: any, res1:any, res2: any, output: any) =>
		{
			const { diff } = require('deep-object-diff');
			
			output += "Headers test : ";
			const headers1 = JSON.parse(JSON.stringify(res1.headers));
			const headers2 = JSON.parse(JSON.stringify(res2.headers));
			const differences = diff(headers2, headers1);
			
			if (Object.keys(differences).length === 0) {
				output += '\x1b[32mSUCCESS\x1b[0m \n';
			} else {
				output += `\x1b[31mFAIL\n\x1b[0mExpected: ${JSON.stringify(res2.headers, null, 2)} | Diff: \x1b[31m${JSON.stringify(differences, null, 2)}}\x1b[0m \n`;
			}
			//Determine headers to compare
			console.log(output);
		}
		const	compareBody = (data1:any, data2: any, res1:any, res2: any, output: any) =>
		{
			// Compare response body
			output += "Compare body test : "
			if (data1 === data2) {
				output += '\x1b[32mSUCCESS\x1b[0m \n';
			} else {
				output += `\x1b[31mFAIL\x1b[0m`;
				//output += `\x1b[31mFAIL\n\x1b[0mExpected: ${data2} \nActual: \x1b[31m${data1}\x1b[0m`;
			}
			console.log(output);
		}
		const	compareCookies = (data1:any, data2: any, res1:any, res2: any, output: any) =>
		{
		
			const cookie1 = res1.headers['set-cookie'];
			const cookie2 = res2.headers['set-cookie'];
			if (JSON.stringify(cookie1) === JSON.stringify(cookie2)) {
			console.log('Cookies match');
			} else {
				console.log('Cookies do not match');
			}
			console.log(output);
		}
		const	compareTime = (data1:any, data2: any, res1:any, res2: any, output: any) =>
		{
			// const timeDiff = Math.abs(new Date(res1.headers.date) - new Date(res2.headers.date));
			// if (timeDiff < 100) {
			// 	console.log('Response times are similar');
			// } else {
			// 	console.log('Response times are different');
			// }
			console.log(output);
		}
		const	compareError = (data1:any, data2: any, res1:any, res2: any, output: any) =>
		{
			// Compare error messages
			// if (res1.statusCode !== 200 && res2.statusCode !== 200) {
			// 	output += "Error message test : \n";
			// 	const error1 = JSON.parse(data1);
			// 	const error2 = JSON.parse(data2);
			// 	if (error1 === error2) {
			// 	output += '\x1b[32mSUCCESS\x1b[0m';
			// 	} else {
			// 		output += `\x1b[31mFAIL\n\x1b[0mExpected: ${error2} \nActual:\x1b[31m${error1}\x1b[0m`;
			// 	}
			// }
			console.log(output);
		}

		const 	compareServers = (testCase: any, currTest: any, server: any) =>
		{
			return new Promise<void>(resolve =>
			{

				const req = http.request(currTest, (res1) => {
					let output: string = "";
					let	data1 = ' ';
					res1.on('data', (chunk) => {
						data1 = chunk;
					});
					res1.on('end', () => {
						currTest["host"] = "localhost";
						currTest["port"] = 80;
						currTest["headers"] = {Host: 'localhost:80'};
						const req2 = http.request(currTest, (res2) => {
							let	data2 = ' ';
							res2.on('data', (chunk2) => {
								data2 = chunk2;
							});
							res2.on('end', () => {
								console.log(`${testCase.name}: \n`);
								compareStatusCode(data1, data2, res1, res2, output);
								compareHeaders(data1, data2, res1, res2, output);
								compareBody(data1, data2, res1, res2, output);
								// compareError(data1, data2, res1, res2, output);
								// compareCookies(data1, data2, res1, res2, output);
								resolve();
							});
						});
						req2.on('error', (e) => {
							console.error(`problem with request: ${e}`);
						});
						req2.end();
					});
					res1.setEncoding('utf8');
				});
				req.on('error', (e) => {
					console.error(`problem with request: ${e}`);
				});
				req.end();
			});
		}
			
		const doTests = async () => {
			
			if (!this.data) {
				throw new Error('Cannot parse configuration file');
			}
			
			//const server = http.createServer((req, res) => {res.end();});
			//server.listen(3000, 'localhost', () => {});
			const server = "";
			let { suiteFixture, testCases } = this.data;
			for (let testCase of testCases)
			{
				let currTest = new TestData();
			
				for (let param in suiteFixture) {
					currTest[param] = suiteFixture[param];
				}	
				for (let param in testCase) {
					currTest[param] = testCase[param];
				}
				await compareServers(testCase, currTest, server);
			}	
			// server.close(()=>{
			// 	console.log("Default Server closed");
			// });
		}

		doTests();
	}
}
