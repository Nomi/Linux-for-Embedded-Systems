#!/usr/bin/php
<?php

$ftp_server = "speedtest.tele2.net";
$ftp_conn = ftp_connect($ftp_server) or die("Could not connect to $ftp_server");
$login = ftp_login($ftp_conn, 'anonymous', '');
if (ftp_put($ftp_conn, "messages-log.txt", "/tmp/messages", FTP_ASCII))
  {
  echo "Successfully uploaded /tmp/messages";
  }
else
  {
  echo "Error uploading /tmp/messages";
  }
  ftp_close($ftp_conn);
#btw, make sure to have execution permission. Also, if permissions causes a problem, just use chmod +x or chown!

 ?>
