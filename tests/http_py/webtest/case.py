from .color import *
from .config import *
import os
import importlib
from pathlib import Path


def run_test(instance, method):
	try:
		result: str = getattr(instance, method)()
	except:
		print(
			"{}Cannot connect to the server on port {}{}".format(C_B_RED, SERVER_PORT, RESET)
		)
		exit(1)
	char = ""
	color = C_GREEN
	if len(result) == 0:
		char = "✅"
	else:
		color = C_RED
		char = "❌"
	print(r"{}.{:20} {}{} {}{}".format(instance.__name__, method[len("test_"):], color, char, result, RESET))


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

	def main(self):
		for key, data in self.case_registry.items():
			for test_name in list(data.test_registry):
				run_test(data.instance, test_name)
			print("")
