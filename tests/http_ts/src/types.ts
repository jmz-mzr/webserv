export class TestData {
	[key: string]: any;
	host?: string;
	port?: number;
	name?: string;
	requestLine?: string;
	headers?: { [key: string]: string };
	body?: string;
	expectedCode?: number;
}

export interface ITestSuite {
	name: string,
	data?: { suiteFixture: TestData, testCases: TestData[] },
	init(): void,
	execute(): void
}

export class Logger {
	message: string;

	constructor() {
		this.message = "";
	}
}