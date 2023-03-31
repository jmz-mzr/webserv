#!/usr/bin/env php-cgi

<?php
	ini_set('session.save_path', dirname($_SERVER['DOCUMENT_ROOT']) . '/sessions');
	if (session_status() !== PHP_SESSION_ACTIVE) session_start();
	phpinfo()
?>