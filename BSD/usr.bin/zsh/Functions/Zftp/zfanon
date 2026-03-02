# function zfanon {

emulate -L zsh

[[ $curcontext = :zf* ]] || local curcontext=:zfanon
local opt opt_1 dir

while getopts :1 opt; do
  [[ $opt = "?" ]] && print "zfanon: bad option: -$OPTARG" >&2 && return 1
  eval "opt_$opt=1"
done
(( OPTIND > 1 )) && shift $(( OPTIND - 1 ))

if [[ -z $EMAIL_ADDR ]]; then
  # Exercise in futility.  There's a poem by Wallace Stevens
  # called something like `N ways of looking at a blackbird',
  # where N is somewhere around 0x14 to 0x18.  Now zftp is
  # ashamed to present `N ways of looking at a hostname'.
  local domain host
  # First, maybe we've already got it.  Zen-like.
  if [[ $HOST = *.* ]]; then
    # assume this is the full host name
    host=$HOST
  elif [[ -f /etc/resolv.conf ]]; then
    # Next, maybe we've got resolv.conf.
    domain=${${=${(M)${(f)"$(</etc/resolv.conf)"}:#domain*}}[2]}
    [[ -n $domain ]] && host=$HOST.$domain
  fi
  # Next, maybe we've got nslookup.  May not work on LINUX.
  [[ -z $host ]] && host=${${=${(M)${(f)"$(nslookup $HOST 2>/dev/null)"}:#Name:*}}[2]}
  if [[ -z $host ]]; then
    # we're running out of ideas, but this should work.
    # after all, i wrote it...
    # don't want user to know about this, too embarrassed.
    local oldvb=$ZFTP_VERBOSE oldtm=$ZFTP_TMOUT
    ZFTP_VERBOSE=
    ZFTP_TMOUT=5
    if zftp open $host >& /dev/null; then
      host=$ZFTP_HOST
      zftp close $host
    fi
    ZFTP_VERBOSE=$oldvb
    ZFTP_TMOUT=$oldtm
  fi
  if [[ -z $host ]]; then
    print "Can't get your hostname.  Define \$EMAIL_ADDR by hand."
    return 1;
  fi
  EMAIL_ADDR="$USER@$host"
  print "Using $EMAIL_ADDR as anonymous FTP password."
fi

if [[ $1 = */* ]]; then
  1=${1##ftp://}
  dir=${1#*/}
  1=${1%%/*}
fi

if [[ $opt_1 = 1 ]]; then
  zftp open $1 anonymous $EMAIL_ADDR || return 1
else
  zftp params $1 anonymous $EMAIL_ADDR
  zftp open || return 1
fi

if [[ -n $dir ]]; then
  zfcd $dir
fi
# }
