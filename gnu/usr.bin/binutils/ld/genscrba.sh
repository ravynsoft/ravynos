#!/bin/bash
source_em()
{
  local current_script="$em_script"
  em_script=$1
  source_sh $1
  em_script=$current_script
}
fragment()
{
  if [ ${BASH_VERSINFO[3]} -ge 3 ]; then
    local lineno=$[${BASH_LINENO[0]} + 1]
    echo >> e${EMULATION_NAME}.c "#line $lineno \"$em_script\""
  fi
  cat >> e${EMULATION_NAME}.c
}
