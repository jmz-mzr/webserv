import sys
import time
import logging
from .color import *
from enum import Enum


class RunStatus(Enum):
    SUCCESS = 0
    FAIL = 1


class TestResult:
    def __init__(self, test_name):
        self.status = RunStatus.SUCCESS
        self.exec_time = -1
        self.message = ""
        self.test_name = test_name

    def console_print(self):
        if self.status == RunStatus.SUCCESS:
            char = "✅"
            color = C_GREEN
        else:
            char = "❌"
            color = C_RED
        print(f"{self.test_name:42} {color}{char} {self.message}{RESET}")


class TestRunner:
    def __init__(self, instance, method):
        self.instance = instance
        self.method = method
        self.function = getattr(instance, method)
        self.result = TestResult(self.instance.__name__ + "." + self.method[len("test_"):])

    def __call__(self):
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
            self.result.console_print()
            return
