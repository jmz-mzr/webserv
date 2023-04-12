from webtest import *
import os

def init_test_folder():
	root_dir = os.environ.get("WORKDIR")
	if not root_dir:
		raise NameError("WORKDIR environment variable undefined")
	upload_dir = os.path.join(root_dir, "www/webserv.test/upload")
	if not os.path.isdir(upload_dir):
		raise FileNotFoundError(f"{upload_dir} not found")
	for filename in os.listdir(upload_dir):
		if filename == ".gitkeep":
			continue
		file_path = os.path.join(upload_dir, filename)
		try:
			if os.path.isfile(file_path) or os.path.islink(file_path):
				os.unlink(file_path)
		except Exception as err:
			print(f"Failed to delete {file_path}. Reason: {err}")
			raise
	

if __name__ == "__main__":
	try:
		init_test_folder()
		runner = TestCase()
		runner.main()
	except Exception as err:
		print(f"Error: {err}")
		exit(1)
