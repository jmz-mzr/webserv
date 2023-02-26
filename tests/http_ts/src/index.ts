import * as path from 'path'
import { readdirSync } from 'fs';
import { ITestSuite } from './types'

const workdir = process.env.INIT_CWD?.toString();
if (!workdir) {
	throw new Error('Cannot find current working directory');
}
let testDir = path.join(workdir, 'build/tests/');

readdirSync(testDir).forEach(file => {
	if (!file.endsWith('.js')) return;
	
	const testSuite: ITestSuite = require(`${testDir}/${file}`).TestSuite;
	console.log(`${testSuite.name}`);
	console.log(new Array(42).join('='));
	testSuite.init();
	testSuite.execute();
});