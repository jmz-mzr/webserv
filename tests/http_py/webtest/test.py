import os
import time
import importlib
from enum import Enum
from pathlib import Path
from threading import Thread, Lock
from .color import *
from .env import *


class RunStatus(Enum):
    SUCCESS = 0
    FAIL = 1


class TestResult:
    test_count = 0
    failed_test_count = 0
    timedout_test_count = 0

    def __init__(self, test_name):
        self.status = RunStatus.SUCCESS
        self.exec_time = -1
        self.message = ""
        self.test_name = test_name

    def __str__(self):
        if self.status == RunStatus.SUCCESS:
            char = "✅"
            color = C_GREEN
        else:
            char = "❌ "
            color = C_RED
        return f"{self.test_name[len('test_')::]:40}{color}{char} {self.message}{RESET}"


class TestRunner:
    def __init__(self, test_fn, test_result):
        self.function = test_fn
        self.result = test_result

    def __call__(self, lock):
        try:
            start = time.time()
            self.result.message = self.function()
            end = time.time()
            self.result.exec_time = end - start
            if len(self.result.message) != 0:
                self.result.status = RunStatus.FAIL
        except Exception as err:
            self.result.message = err
            self.result.status = RunStatus.FAIL
        finally:
            if self.result.status == RunStatus.FAIL:
                with lock:
                    TestResult.failed_test_count += 1


class TestData:
    def __init__(self, instance, test_registry: dict):
        self.case_name = instance.__name__
        self.instance = instance
        self.test_registry = test_registry
        self.test_nb = len(self.test_registry)
        self.test_result = {test: TestResult(test) for test in list(test_registry)}

    def __repr__(self):
        return "Case {} : {} test{} registered".format(self.case_name, self.test_nb, "s" if self.test_nb > 1 else "")


class TestCase:
    case_registry = dict()

    def __init_subclass__(cls):
        """
        Register classes inheriting from TestCase and all the test methods they contain
        """
        super().__init_subclass__()
        tests = dict()
        if cls.__name__ in cls.case_registry.keys():
            raise Exception(f'"{cls.__name__}" test case already exists')
        for attribute in dir(cls):
            if callable(getattr(cls, attribute)) and attribute.startswith('test') is True:
                tests[attribute] = getattr(cls, attribute)
                TestResult.test_count += 1
        cls.case_registry[cls.__name__] = TestData(cls, tests)

    def __init__(self):
        path = Path(os.path.dirname(__file__))
        for file in os.listdir(os.path.join(path.parent.absolute(), MODULE_DIR)):
            if file.endswith(".py") and not file.startswith("_"):
                module_name = file[:-len(".py")]
                importlib.import_module("modules." + module_name, ".")

    def run(self) -> None:
        threads = list()
        lock = Lock()
        for data in self.case_registry.values():
            for test_name, test_fn in data.test_registry.items():
                runner = TestRunner(test_fn, data.test_result[test_name])
                t = Thread(target=runner, args=[lock])
                threads.append(t)
                t.start()
        for thread in threads:
            thread.join()
