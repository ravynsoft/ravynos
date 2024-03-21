# bracketed-paste-url-magic quotes pasted urls automatically, if the
# paste exactly starts with a url, eg no spaces or other characters precede it
#
# If the numeric argument is provided (eg, pressing alt-0 or alt-1 in emacs mode,
# or just the number by itself in vi command mode), then
# 0 is the default and means auto detect urls
# 1 means always quote
# any other value means never quote
#
# To use this widget, put this in your startup files (eg, .zshrc)
#
# autoload -Uz bracketed-paste-url-magic
# zle -N bracketed-paste bracketed-paste-url-magic
#
# You can customize which schemas are to be quoted by using
#
# zstyle :bracketed-paste-url-magic schema http https ftp
#
# The default can be seen just below.

local -a schema
zstyle -a :bracketed-paste-url-magic schema schema || schema=(http:// https:// ftp:// ftps:// file:// ssh:// sftp:// magnet:)

local wantquote=${NUMERIC:-0}
local content
local start=$#LBUFFER

zle .$WIDGET -N content

if (( $wantquote == 0 )); then
  if [[ $content = (${(~j:|:)schema})* ]]; then
    wantquote=1
  fi
fi

if (( $wantquote == 1 )); then
  content=${(q-)content}
fi

LBUFFER+=$content

YANK_START=$start
YANK_END=$#LBUFFER
zle -f yank
