# Function to find a web browser to run on a URL or file.
# Can also be run as a script.  It is suitable for use as
# a suffix alias:
#   alias -s html=pick-web-browser
#
# The single argument is the URL or file name which may be of any type.
# The only processing which occurs is that if the argument is a file,
# it is converted into a URL.  As the function takes account of
# any necessary conversions to the file name (for example, if it
# contains spaces), it is generally preferable to pass in raw file
# names rather than convert them to URLs elsewhere.
#
# The function takes account of the fact that many X Windows browsers
# which are already running on the current display can take a command
# to pass the URL to that process for handling.  A typical sign
# that this has happened is that apparently nothing happens --- you
# need to check the browser window.
#
# If no $DISPLAY is set, the function tries to start a terminal-based
# browser instead.

emulate -L zsh
setopt extendedglob cbases nonomatch warncreateglobal

zmodload -i zsh/zutil

local -a xbrowsers ttybrowsers

# X Windows browsers which might be running and can accept
# a remote URL.
zstyle -a :mime: x-browsers xbrowsers ||
  xbrowsers=(firefox mozilla netscape opera konqueror)
# Preferred command line browsers.
zstyle -a :mime: tty-browsers ttybrowsers ||
  ttybrowsers=(elinks links lynx)
# Characters in addition to alphanumerics which can appear literally
# in a URL.  `-' should be the first if it appears, so append others
# to the end.
local litc="-_./"

local -a windows remoteargs match mbegin mend
local url browser command

url=$1
if [[ -f $url ]]; then
  if [[ $url = *[^-_[:alnum:]]* ]]; then
    # Convert special characters into hex escapes.
    local sofar
    while [[ $url = (#b)([${litc}[:alnum:]]#)([^${litc}[:alnum:]])(*) ]]
      do
      sofar+="$match[1]%${$(( [#16] ##$match[2] ))##0x}"
      url=$match[3]
    done
    url="$sofar$url"
  fi

  # Turn this into a local URL
  if [[ $url = /* ]]; then
      url=file://$url
  else
      url=file://$PWD/$url
  fi
fi

local bstyle
local -a bstyles
zstyle -a :mime: browser-styles bstyles || bstyles=(running x tty)

for bstyle in $bstyles; do
  case $bstyle in
    (running)
    [[ -z $DISPLAY ]] && continue
    # X Windows running

    # Get the name of all windows running; use the internal name, not
    # the friendly name, which is less useful.

    windows=(${(ou)${(M)${(f)"$(xwininfo -root -all)"}:#*\"*\"\:[[:space:]]\(\"*}/(#b)*\"*\"\:[[:space:]]\(\"(*)\"[[:space:]]\"*\"\)*/$match[1]})
    #windows=(${(f)"$(xwininfo -root -all |
    #     perl -ne '/.*"(.*)": \("(.*)" "(.*)"\).*/ and $w{$2} = 1;
    #               END { print join("\n", keys %w), "\n" }')"})

    # Is any browser we've heard of running?
    for browser in $xbrowsers; do
      # Some browser executables call themselves <browser>-bin
      if [[ $windows[(I)(#i)$browser(|[.-]bin)] -ne 0 ]]; then
	if zstyle -s ":mime:browser:running:${browser}:" command command; then
	  # The (q)'s here and below are pure paranoia:  no browser
	  # name is going to include metacharacters, and we already
	  # converted difficult characters in the URL to hex.
	  zformat -f command $command b:${(q)browser} u:${(q)url}
	  eval $command
	else
	  case $browser in
	    (konqueror)
	    # kfmclient is less hairy and better supported than direct
	    # use of dcop.  Run kfmclient --commands
	    # for more information.  Note that as konqueror is a fully
	    # featured file manager, this will actually do complete
	    # MIME handling, not just web pages.
	    kfmclient openURL $url ||
	    dcop $(dcop|grep konqueror) default openBrowserWindow $url
	    ;;

	    (firefox)
	    # open in new tab
	    $browser -new-tab $url
	    ;;

	    (opera)
	    $browser -newpage $url
	    ;;

	    (*)
	    # Mozilla bells and whistles are described at:
	    # http://www.mozilla.org/unix/remote.html
	    $browser -remote "openURL($url)"
	    ;;
	  esac
	fi
	return
      fi
    done
    ;;

    (x)
    [[ -z $DISPLAY ]] && continue
    # Start our preferred X Windows browser in the background.
    for browser in $xbrowsers; do
      if eval "[[ =$browser != \\=$browser ]]"; then
	if zstyle -s ":mime:browser:new:${browser}:" command command; then
	  zformat -f command $command b:${(q)browser} u:${(q)url}
	  eval $command "&"
	else
	  # The following is to make the job text more readable.
	  eval ${(q)browser} ${(q)url} "&"
	fi
	return
      fi
    done
    ;;

    (tty)
    # Start up dumb terminal browser.
    for browser in $ttybrowsers; do
      if eval "[[ =$browser != \\=$browser ]]"; then
	if zstyle -s ":mime:browser:new:${browser}" command command; then
	  zformat -f command $command b:${(q)browser} u:${(q)url}
	  eval $command
	else
	  $browser $url
	fi
	return
      fi
    done
    ;;
  esac
done

# No eligible browser.
return 255
