export class TestData {
	[key: string]: any;
	host?: string;
	port?: number;
	name?: string;
	requestLine?: string;
	headers?: string[];
	body?: string;
	expectedCode?: number;
}

export interface ITestSuite {
	name: string,
	data?: { suiteFixture: TestData, testCases: TestData[] },
	init(): void,
	execute(): void
}