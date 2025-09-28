#! /bin/bash

case "$1" in
  *.exe)
    fccwd=`pwd`
    cd $(IFS=:;for i in $PATH; do echo $i|grep mingw> /dev/null; [ $? -eq 0 ] && echo $i; done)
    /usr/bin/env wine $fccwd/$@
    ;;
  *)
    $@
    ;;
esac

