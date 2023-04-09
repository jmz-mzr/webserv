#!/usr/bin/env php-cgi
<?php

function html_body(string $str, bool $file_saved = false) {
	echo "<!DOCTYPE html>\n<html>\n";
	if ($file_saved)
		echo "<head><title>Your file has been saved!</title></head>\n";
	echo "<body>\n<center><h2>{$str}</h2></center>\n";
	echo "<hr><center>webserv</center>\n";
	echo "</body>\n</html>\n";
}

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
	http_response_code(403);
	html_body("Error: invalid request method, only POST is allowed");
	exit();
}

$content_type = $_SERVER['CONTENT_TYPE'] ?? '';
$regex = '/(^multipart\/form-data)[[:space:]]*;[[:space:]]*(boundary=)';
$regex .= '(".*"|[^"].*[^"])/';

if (preg_match($regex, $content_type) == 1) {
	if (!isset($_FILES['file']) || $_FILES['file']['error'] !== UPLOAD_ERR_OK) {
		http_response_code(400);
		html_body("Error: invalid file");
		exit();
	}
	$upload_dir = $_SERVER['DOCUMENT_ROOT'];
	$filename = $_FILES['file']['name'];
	if (file_exists($upload_dir . $filename)) {
		http_response_code(409);
		html_body("Error: the file \"$filename\" already exists");
		exit();
	}
	if (!move_uploaded_file($_FILES['file']['tmp_name'],
								$upload_dir . $filename)) {
		http_response_code(500);
		html_body("Error: Failed to upload file");
		exit();
	}
} else {
	$file_path = $_SERVER['PATH_TRANSLATED'];
	if (is_dir($file_path) || substr($file_path, -1) === '/') {
		if (substr($file_path, -1) !== '/')
			$file_path .= '/';
		$file_path .= date("y-m-d_H:i:s_") . uniqid();
	}
	if (file_exists($file_path)) {
		http_response_code(409);
		html_body("Error: the file \"{$_SERVER['PATH_INFO']}\" already exists");
		exit();
	}
	$file_data = file_get_contents('php://input');
	if (file_put_contents($file_path, $file_data) === false) {
		http_response_code(500);
		html_body("Error: Failed to upload file");
		exit();
	}
}

html_body("Your file has been saved! Click "
	. "<A href=\"{$_SERVER['PATH_INFO']}\">here</A> to view it.", true);

?>
