import * as path from 'path'
import * as fs from 'fs';
import { ITestSuite } from './types'

const workdir = process.env.INIT_CWD?.toString();
if (!workdir) {
	throw new Error('Cannot find current working directory');
}
let testDir = path.join(workdir, 'build/modules/');

let files = fs.readdirSync(testDir);
for (let file of files) {
	if (!file.endsWith('.js')) continue;
	
	const testSuite: ITestSuite = require(`${testDir}/${file}`).TestSuite;

	console.log(`${testSuite.name}`);
	console.log(new Array(42).join('='));

	testSuite.init();
	testSuite.execute();
}