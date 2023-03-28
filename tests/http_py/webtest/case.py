from .config import *
from .run import TestRunner
import os
import importlib
from pathlib import Path


class TestData:
	def __init__(self, instance, test_registry):
		self.instance = instance
		self.test_registry = test_registry


class TestCase:
	case_registry = dict()

	def __init_subclass__(cls):
		"""
		Register classes inheriting from TestCase and all the test methods they contain
		"""
		super().__init_subclass__()
		tests = dict()
		for attribute in dir(cls):
			if callable(getattr(cls, attribute)) and attribute.startswith('test') is True:
				tests[attribute] = getattr(cls, attribute)
		cls.case_registry[cls.__name__] = TestData(cls, tests)

	def __init__(self):
		path = Path(os.path.dirname(__file__))
		for file in os.listdir(os.path.join(path.parent.absolute(), MODULE_DIR)):
			if file.endswith(".py") and not file.startswith("_"):
				module_name = file[:-len(".py")]
				importlib.import_module("modules." + module_name, ".")

	def main(self) -> None:
		for data in self.case_registry.values():
			for test_name in list(data.test_registry):
				runner = TestRunner(data.instance, test_name)
				result = runner()
				result.console_print()
			print("")