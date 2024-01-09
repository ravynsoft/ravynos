# Call this from the preexec function like so:
#   preexec() {
#	  getjobs "${(z)2}"  # Use "${(z)1}" prior to zsh-4.0.1
#   }
setopt localoptions noshwordsplit noksharrays
local texts
case $1 in
    fg|bg) shift; [[ -n $1 ]] || set -- %% ;;
    %*) ;;
    *) return 0 ;;
esac
repeat $#
do
    # This case statement emulates jobs.c:getjob()
    case $1 in
	[\;\&\|]|\|\||\&\&) break ;;
	%(|[%+])) 1=${(k)jobstates[(r)*:+:*]} ;;
	%-) 1=${(k)jobstates[(r)*:-:*]} ;;
	%<->) 1=${1#%} ;;
	%[?]*) 1=${${(Ok)jobtexts[(R)*${1#%[?]}*]}[1]} ;;
	*) 1=${${(Ok)jobtexts[(R)$1*]}[1]} ;;
    esac
    [[ -n $1 ]] && texts=($texts ${jobtexts[$1]})
    shift
done
# Remove the "-s" below if you'd prefer that this just report
# what jobs are being affected rather than modify the history
(( $#texts )) && print -s ${(j:; :)texts} "$*"
return 0
