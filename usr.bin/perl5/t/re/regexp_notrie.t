#!./perl
#use re 'debug';
BEGIN {
    ${^RE_TRIE_MAXBUF}=-1;
    #${^RE_DEBUG_FLAGS}=0;
}

$qr = 1;
for $file ('./re/regexp.t', './t/re/regexp.t', ':re:regexp.t') {
    if (-r $file) {
	do $file or die $@;
	exit;
    }
}
die "Cannot find ./re/regexp.t or ./t/re/regexp.t\n";
