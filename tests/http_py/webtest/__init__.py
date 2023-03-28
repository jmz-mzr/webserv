"""
Simple testing framework for 42 school webserv project

Usage:

    from webtest import *

    class RandomTest(webtest.TestCase):
        def testAdd(self):  # test method names begin with 'test'
            if 1 + 1 != 2:
                return "Bad result: {0:d}, expected: {1:d}".format(1 + 1, 2)    # return an error message if test fails
            return ""   # return an empty string if success

    if __name__ == '__main__':
        runner = TestCase()
        runner.main()
"""

from .case import TestCase
from .utils import send_request
from .utils import print_headers
from .config import *