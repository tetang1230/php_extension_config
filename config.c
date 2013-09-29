/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2012 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include "php.h"
#include "php_ini.h"
#include "main/SAPI.h"
#include "ext/standard/php_string.h"
#include "php_load_config.h"

ZEND_DECLARE_MODULE_GLOBALS(config)

/* True global resources - no need for thread safety here */
static int le_config;

/* {{{ config_functions[]
 *
 * Every user visible function must have an entry in config_functions[].
 */
const zend_function_entry config_functions[] = {
	PHP_FE(config,	NULL)		/* For testing, remove later. */
	{ NULL, NULL, NULL, 0, 0 }	/* Must be the last line in config_functions[] */
};
/* }}} */

/* {{{ config_module_entry
 */
zend_module_entry config_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"config",
	config_functions,
	PHP_MINIT(config),
	PHP_MSHUTDOWN(config),
	PHP_RINIT(config),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(config),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(config),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_CONFIG
ZEND_GET_MODULE(config)
#endif

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("config.path", "", PHP_INI_SYSTEM, OnUpdateString, path, zend_config_globals, config_globals)
PHP_INI_END()

/* }}} */

static void php_config_init_globals(zend_config_globals *config_globals)
{
	config_globals->path = NULL;
	config_globals->slist = NULL;
}

static ulong time33(const char *arKey, uint nKeyLength)
{
	register ulong hash = 5381;

	/* variant with the hash unrolled eight times */
	for (; nKeyLength >= 8; nKeyLength -= 8) {
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
		hash = ((hash << 5) + hash) + *arKey++;
	}
	switch (nKeyLength) {
		case 7: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
		case 6: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
		case 5: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
		case 4: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
		case 3: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
		case 2: hash = ((hash << 5) + hash) + *arKey++; /* fallthrough... */
		case 1: hash = ((hash << 5) + hash) + *arKey++; break;
		case 0: break;
EMPTY_SWITCH_DEFAULT_CASE()
	}
	return hash;
}

/**
 * 检测key是否已经存在
 * @param  slist 单项列表
 * @param  key   键名
 * @param  key_len 键名长度
 * @return int 大于0表示已经存在，否则返回0
 */
static int slist_key_exists(SLIST *slist, char *key, int key_len){
	node *cur;

	if(slist->length == 0) return 0;

	cur = slist->first;

	while(cur != NULL){

		if(cur->hashcode == key_len && strcmp(cur->key, key) == 0){
			return 1;
		}

		cur = cur->next;
	}

	return 0;
}

static int slist_add_item(SLIST *slist, char *key, int key_len, char *val, int val_len){
	node *cur, *last;

	if(slist_key_exists(slist, key, key_len)){
		return 0;
	}

	cur = (node *)pemalloc(sizeof(node), 1);
	cur->next = NULL;
	cur->key  = pestrdup(key, 1);
	cur->hashcode = time33(key, key_len);
	cur->data = pestrdup(val, 1);

	if(slist->length == 0){
		slist->first = cur;
		slist->last = cur;
	}else{
		last = slist->last;
		while(1){
			if(last->next == NULL){
				last->next = cur;
				break;
			}else{
				last = last->next;
			}
		}
	}

	slist->length++;

	return 1;
}

static int slist_find_item(SLIST *slist, char *key, int key_len, char **data){
	node *cur;

	if(slist->length == 0) return 0;

	cur = slist->first;

	while(cur != NULL){

		if(cur->hashcode == time33(key, key_len) && strcmp(cur->key, key) == 0){
			*data = cur->data;
			return SUCCESS;
		}

		cur = cur->next;
	}

	return FAILURE;

}

static int php_config_initlize(zval *file_path, int module_number){
	char content[1024] = {0};
	FILE *fp;
	int file_exists;
	SLIST *slist;
	struct stat file_stat;
	int action = CONFIG_TYPE_UNKONW; //0:定义define, 1:定义array, -1:不操作

	slist 		= CONFIG_G(slist);
	file_exists = VCWD_STAT(Z_STRVAL_P(file_path), &file_stat);

	if(file_exists != 0){
		zend_error(E_WARNING, "file:%s not exists!", Z_STRVAL_P(file_path));
		return 1;
	}

	fp = VCWD_FOPEN(Z_STRVAL_P(file_path), "r");

	while(fgets(content, 1024, fp)){
		char *trim_content, *temp_content;
		char *p;

		temp_content = estrndup(content, strlen(content)+1);
		trim_content = php_trim(temp_content, strlen(temp_content), NULL, 0, NULL, 3 TSRMLS_CC);

		if(strlen(trim_content) == 0 || trim_content[0] == ';') {
			efree(temp_content);
			continue;
		}

		p = strstr(trim_content, "=");

		if(!p && strcmp(trim_content, "[define]") == 0){
			action = CONFIG_TYPE_DEFINE;
		}else if(!p &&  strcmp(trim_content, "[config]") == 0){
			action = CONFIG_TYPE_CONFIG;
		}else if(p){
			char *key;
			char *trim_key, *trim_val;
			int trim_val_len, trim_key_len;
			zend_constant c;

			key = estrndup(trim_content, strlen(trim_content) - strlen(p));
			trim_val 	= p+1;
			trim_val_len= strlen(trim_val);

			trim_key = php_trim(key, strlen(key), NULL, 0, NULL, 3 TSRMLS_CC);

			trim_val = php_trim(trim_val, trim_val_len, NULL, 0, NULL, 3 TSRMLS_CC);
			trim_val_len = strlen(trim_val);
			trim_key_len = strlen(trim_key);

			switch(action){
				//未定义操作 
				case CONFIG_TYPE_UNKONW:
					break;
				case CONFIG_TYPE_DEFINE:
						c.value.type = IS_STRING;
						c.value.value.str.val = pestrdup(trim_val, trim_val_len+1);
						c.value.value.str.len = trim_val_len;
						c.flags = CONST_PERSISTENT | CONST_CS;
						c.name = pestrdup(trim_key, trim_key_len+1);
						c.name_len = trim_key_len+1;
						c.module_number = module_number;
						zend_register_constant(&c TSRMLS_CC);
					break;
				case CONFIG_TYPE_CONFIG:
						slist_add_item(slist, trim_key, trim_key_len, trim_val, trim_val_len);
					break;
			}

			efree(key);
			efree(trim_val);
			efree(trim_key);

		}
		// free(p);
		efree(trim_content);
		efree(temp_content);
	}
}

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(config)
{
	zval *delim, *explode_arr, *explode_strs;
	HashTable *files;
	php_config_init_globals(&config_globals);
	REGISTER_INI_ENTRIES();

	config_globals.slist = (SLIST *)pemalloc(sizeof(SLIST), 1);
	config_globals.slist->last  = NULL;
	config_globals.slist->first = NULL;
	config_globals.slist->length= 0;

	MAKE_STD_ZVAL(delim);
	MAKE_STD_ZVAL(explode_strs);
	MAKE_STD_ZVAL(explode_arr);

	array_init(explode_arr);
	ZVAL_STRING(delim, ",", 1);
	ZVAL_STRING(explode_strs, CONFIG_G(path), 1);

	php_explode(delim, explode_strs, explode_arr, LONG_MAX);

	files = Z_ARRVAL_P(explode_arr);

	for (
		zend_hash_internal_pointer_reset(files); 
		zend_hash_has_more_elements(files) == SUCCESS; 
		zend_hash_move_forward(files)
	){
		zval **file_path;

		if(zend_hash_get_current_data(files, (void**)&file_path) == FAILURE){
			continue;
		}

		if(Z_STRLEN_PP(file_path) > 0){
			php_config_initlize(*file_path, module_number);
		}
	}

	zend_hash_destroy(Z_ARRVAL_P(explode_arr));
	efree(Z_STRVAL_P(explode_strs));
	efree(Z_STRVAL_P(delim));
	efree(delim);
	efree(explode_strs);
	efree(explode_arr);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(config)
{
	/* uncomment this line if you have INI entries*/
	UNREGISTER_INI_ENTRIES();
	
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(config)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(config)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(config)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "config support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini */
	DISPLAY_INI_ENTRIES();
	
}
/* }}} */


/**
 * 读取或设置配置
 * @param string $key  配置键名
 * function config(string $key);
 */
PHP_FUNCTION(config)
{
	char *key = NULL;
	int key_len;
	SLIST *slist;
	char *data;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &key_len) == FAILURE) {
		return;
	}

	slist = CONFIG_G(slist);

	if(slist_find_item(slist, key, key_len, &data) == SUCCESS){
		RETURN_STRING(data, 1);
	}

	RETURN_NULL();
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
