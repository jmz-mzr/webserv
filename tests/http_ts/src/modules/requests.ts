import * as path from 'path'
import { TestData, ITestSuite } from '../types'

export const TestSuite: ITestSuite = {
	name: "requests",
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
	}
}