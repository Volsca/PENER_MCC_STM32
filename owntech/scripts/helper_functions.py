import os
import platform
import zipfile
import certifi
import shutil
from stat import *

def unzip_file(zip_file_path, extract_to_path):
    with zipfile.ZipFile(zip_file_path, 'r') as zip_ref:
        zip_ref.extractall(extract_to_path)

def check_file_and_download(what, folder_path, file_name, file_url, make_exec = False):
	# Make sure folder exists
	if not os.path.isdir(folder_path):
		os.makedirs(folder_path)
	if not os.path.isdir(folder_path):
		print(f"Error! Unable to create directory {folder_path}.")
		print("Make sure you have permission to write in this directory.")
		return -1

	# Determine file path
	file_path = os.path.join(folder_path, file_name)

	# Make sure file is available locally
	if not os.path.isfile(file_path):
		import urllib.request
		from urllib.error import URLError

		print(f"{what} is not available locally, downloading.")
		try:
			# Download file
			ssl_context = urllib.request.ssl.create_default_context(cafile=certifi.where())
			with urllib.request.urlopen(file_url, context=ssl_context) as response, open(file_path, 'wb') as out_file:
				shutil.copyfileobj(response, out_file)
			print("Download complete.")
		except URLError:
			print(f"Error! Unable to download {what}.")
			print("Make sure you are connected to the Internet.")
			return -1

	if make_exec:
		if platform.system() in ['Linux', 'Darwin']:			
			# Make file executable
			st = os.stat(file_path)
			os.chmod(file_path, st.st_mode | S_IXUSR | S_IXGRP | S_IXOTH)
	return 0
