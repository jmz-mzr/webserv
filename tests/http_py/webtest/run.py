import sys
import time
from .color import *
from enum import Enum


class RunStatus(Enum):
    SUCCESS = 0
    FAIL = 1
    TIMEOUT = 2


class TestResult:
    def __init__(self, test_name):
        self.status = RunStatus.SUCCESS
        self.exec_time = -1
        self.message = ""
        self.test_name = test_name

    def console_print(self):
        color = C_RED
        char = "❌"
        if self.status == RunStatus.SUCCESS:
            char = "✅"
            color = C_GREEN
        print(r"{:42} {}{} {}{}".format(self.test_name, color, char, self.message, RESET))


class TestRunner:
    def __init__(self, instance, method):
        self.instance = instance
        self.method = method
        self.function = getattr(instance, method)
        self.result: TestResult = TestResult(self.instance.__name__ + "." + self.method[len("test_"):])
        pass

    def __call__(self) -> TestResult:
        try:
            start = time.time()
            self.result.message = self.function()
            end = time.time()
            self.result.exec_time = end - start
        except TimeoutError:
            self.result.status = RunStatus.TIMEOUT
            self.message = "Timeout"
        except Exception as err:
            print(f"Unexpected {err=}, {type(err)=}")
            sys.exit(1)
        finally:
            if len(self.result.message) != 0:
                self.result.status = RunStatus.FAIL
            return self.result
