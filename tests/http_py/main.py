from webtest import *
import os


def init_test_folder():
	root_dir = os.environ.get("PREFIX")
	if not root_dir:
		raise NameError("PREFIX environment variable undefined")
	upload_dir = os.path.join(root_dir, "var/www/webserv/webserv.test/upload")
	if not os.path.isdir(upload_dir):
		raise FileNotFoundError(f"{upload_dir} not found")
	for filename in os.listdir(upload_dir):
		if filename == ".gitkeep":
			continue
		file_path = os.path.join(upload_dir, filename)
		try:
			if os.path.isfile(file_path) or os.path.islink(file_path):
				os.unlink(file_path)
		except Exception as error:
			print(f"Failed to delete {file_path}. Reason: {error}")
			raise


def print_result(case_registry):
	for key, value in suite.case_registry.items():
		print(f"{'':=>42}")
		print(f"{key.upper():>42}")
		print(f"{'':=>42}")
		for test_name in list(value.test_registry):
			print(value.test_result[test_name])
		print()


if __name__ == "__main__":
	try:
		init_test_folder()
		suite = TestCase()
		suite.run()
		print_result(suite.case_registry)
	except Exception as err:
		print(f"Error: {err}")
		exit(1)
	finally:
		exit(TestResult.failed_test_count)
