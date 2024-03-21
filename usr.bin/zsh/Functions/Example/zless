#!/bin/zsh -f
#
# zsh function script to run less on various inputs, decompressing as required.
# Author: Phil Pennock.  zsh-hacks@athenaeum.demon.co.uk
# Modified by Bart Schaefer.
# Thanks to zefram@fysh.org for a great deal of help in sorting this out,
# ie wrt syntax for unsetting members of arrays and eval "$(...)" when I
# asked for something better than . =(...)
#
# Use -zforce to pass through a display-formatting command
#  zless -zforce 'bzip2 -dc' foo-no-dotbz2
#  zless -zforce 'od -hc' foo-binfile
#
# If you can understand all of this without reference to zshexpn(1)
# and zshparam(1) then you either have a photographic memory or you
# need to get out more.
#

emulate -R zsh
setopt localoptions

[[ $# -ge 1 ]] || return
local lessopts
set -A lessopts
integer i=1 loi=1
while ((i <= $#))
do
  case $argv[i] in
  -zforce) argv[i,i+2]=("=($argv[i+1] \"$argv[i+2]\")"); ((++i));;
  -*) lessopts[loi++]=\"$argv[i]\"; argv[i]=(); continue;;
  *.(gz|Z)) argv[i]="=(zcat \"$argv[i]\")";;
  *.bz2) argv[i]="=(bzip2 -dc \"$argv[i]\")";;
  *.bz) argv[i]="=(bzip -dc \"$argv[i]\")";;
  esac
  ((++i))
done
eval command less $lessopts $*
