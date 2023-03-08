import * as fs from 'fs'
import * as path from 'path'
import * as net from 'net'
import { parseState, Request, Response } from '../http'
import { TestData, ITestSuite } from '../types'
import test from 'node:test'

export const TestSuite: ITestSuite = {
	name: "request_parsing",
	init() {
		const workdir = process.env.INIT_CWD?.toString();
		if (!workdir) {
			throw new Error('Cannot find current working directory');
		}
		const filePath: string = path.join(workdir, 'data/request_parsing.json');
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

		for (let testCase of testCases) {
			for (let param in testCase) {
				currTest[param] = testCase[param];
			}
			
			let socket: net.Socket;
			if (!currTest.port) {
				throw new Error('No port specified');
			}

			socket = net.createConnection(currTest.port, currTest.host);
			socket.setTimeout(3000);

			const request = new Request(currTest.requestLine, currTest.headers, currTest.body);
			let response = new Response();

			socket.on('connect', () => {
				socket.write(request.message);
			});
			socket.on('data', (data) => {
				try {
					process.stdout.write(`${testCase.name}: `);
					response.parse(data.toString());
				} catch (error) {
					console.error(`\x1b[31m${(error as Error).message}\x1b[0m`);
				} finally {
					if (response.state === parseState.parsed) {
						let output;
						if (testCase.expectedCode === response.statusCode) {
							output = `\x1b[32mSUCCESS\x1b[0m`;
						} else {
							output = `\x1b[31mFAIL\x1b[0m Expected: ${testCase.expectedCode} | Actual: \x1b[31m${response.statusCode}\x1b[0m`;
						}
						console.log(output);
					}
					socket.end();
				}
			});
			socket.on('timeout', () => {
				console.log(`\x1b[31mTIMEOUT\x1b[0m`);
				socket.end();
			  });
			socket.on('error', () => {
				console.error(`\x1b[31mERROR\x1b[0m`)
			});
			socket.on('end', () => {
				// console.log('disconnected from server');
			});
		}
	}
}