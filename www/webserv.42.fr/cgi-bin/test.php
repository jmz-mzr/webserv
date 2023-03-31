<?php

	$opts = array('http' =>

		array(

			'method'  => 'POST',

			'header'  => "Content-Type: application/x-www-form-urlencoded\r\n" . 
						"Host: localhost:8080\r\n" .
						"Content-Length: 24\r\n" .
						"Cookie: PHPSESSID=8h6coqqebenqn9omnleh07ajqu\r\n",

			'content' => "username=test&updatebtn=\r\n",

			'timeout' => 60

		)

	);

	$context  = stream_context_create($opts);

	$url = 'http://localhost:8080';

	$result = file_get_contents($url, false, $context);

	echo $result;

?>