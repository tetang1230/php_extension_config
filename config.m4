dnl $Id$
dnl config.m4 for extension config

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(config, for config support,
dnl Make sure that the comment is aligned:
dnl [  --with-config             Include config support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(config, whether to enable config support,
dnl Make sure that the comment is aligned:
[  --enable-config           Enable config support])

if test "$PHP_CONFIG" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-config -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/config.h"  # you most likely want to change this
  dnl if test -r $PHP_CONFIG/$SEARCH_FOR; then # path given as parameter
  dnl   CONFIG_DIR=$PHP_CONFIG
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for config files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       CONFIG_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$CONFIG_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the config distribution])
  dnl fi

  dnl # --with-config -> add include path
  dnl PHP_ADD_INCLUDE($CONFIG_DIR/include)

  dnl # --with-config -> check for lib and symbol presence
  dnl LIBNAME=config # you may want to change this
  dnl LIBSYMBOL=config # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $CONFIG_DIR/lib, CONFIG_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_CONFIGLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong config lib version or lib not found])
  dnl ],[
  dnl   -L$CONFIG_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(CONFIG_SHARED_LIBADD)

  PHP_NEW_EXTENSION(config, config.c, $ext_shared)
fi
