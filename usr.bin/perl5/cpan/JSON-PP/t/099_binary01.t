$0=~/binary(\d\d)/ or die "Could not detect chunk from '$0'";
$ENV{JSONPP_CHUNK} = 0+$1;
do "./t/099_binary.pl";
