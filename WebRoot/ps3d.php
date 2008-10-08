<?php

define("RESOURCE_DIR", "./PS3D_Resources/");

$mode = $_GET["mode"];
$filename = $_GET["filename"];

$userfile = $HTTP_POST_FILES['userfile']['tmp_name'];
$userfile_name = $HTTP_POST_FILES['userfile']['name'];
$userfile_size = $HTTP_POST_FILES['userfile']['size'];
$userfile_type = $HTTP_POST_FILES['userfile']['type'];
$userfile_error = $HTTP_POST_FILES['userfile']['error'];

$filepath = RESOURCE_DIR . $filename;

if ( $mode == "download" )
	getfile( $filepath );
if ( $mode == "listscene" )
	listscene();
if ( $mode == "upload")
	upload( );
		
function getfile($file)
{
	$fp = fopen($file, "rb");
	
	if( $fp == "" ){
		echo "file not exist";
		exit();
	}
	
	fpassthru($fp);
	fclose($fp);
}

function listscene()
{
	$scene_dir = RESOURCE_DIR . $filename . "Scenes/";
	$dir = opendir($scene_dir);

	while ($file = readdir($dir))
	{
		if ($file != "." && $file != "..")
			echo "$file\n";
	}

	closedir($dir);
}

function upload( )
{
	global $userfile;
	global $userfile_name;
	global $userfile_size;
	global $userfile_type;
	global $userfile_error;
	
 if ( $userfile_error > 0)
  {
    echo 'Problem: ';
    switch ($userfile_error)
    {
      case 1:  echo 'File exceeded upload_max_filesize';  break;
      case 2:  echo 'File exceeded max_file_size';  break;
      case 3:  echo 'File only partially uploaded';  break;
      case 4:  echo 'No file uploaded';  break;
    }
    exit;
  }

  $upfile = './PS3D_Resources/Scenes/'.$userfile_name;

  if (is_uploaded_file($userfile)) 
  {
      if (!move_uploaded_file($userfile, $upfile))
     {
        echo 'Problem: Could not move file to destination directory';
        exit;
     }
  } 
  else 
  {
    echo 'Problem: Possible file upload attack. Filename: '.$userfile_name;
    exit;
  }

  echo 'File uploaded successfully<br /><br />'; 
}

?>