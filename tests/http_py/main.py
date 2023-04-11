from webtest import *
import os

def init_test_folder():
	root_dir = os.environ.get("WORKDIR")
	if not root_dir:
		print("WORKDIR environment variable not defined")
		exit(1)
	upload_dir = os.path.join(root_dir, "www/webserv.test/upload")
	if not os.path.isdir(upload_dir):
		print("Can't find upload directory")
		exit(1)
	for filename in os.listdir(upload_dir):
		if filename == ".gitkeep":
			continue
		file_path = os.path.join(upload_dir, filename)
		try:
			if os.path.isfile(file_path) or os.path.islink(file_path):
				os.unlink(file_path)
		except Exception as err:
			print(f"Failed to delete {file_path}. Reason: {err}")
	

if __name__ == "__main__":
	init_test_folder()
	runner = TestCase()
	runner.main()
