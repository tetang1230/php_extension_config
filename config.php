<?php
$module = 'config';

if (extension_loaded($module)) {
	$str = $function($module);
} else {
	echo "Module $module is not compiled into PHP\n";
	exit;
}

if(!function_exists('config')){
	echo "config function error\n";
}

//读取配置选项config
echo config('base_url') . "\n";

//调用define选项
echo MSG_SERVICE . "\n";
?>
