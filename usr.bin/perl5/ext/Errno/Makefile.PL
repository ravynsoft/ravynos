use ExtUtils::MakeMaker;
use Config;

my $arch = "$Config{'archname'}-$Config{'osvers'}";
my $got = "";
if (-e 'arch.txt') {
    open my $in, "<", "arch.txt" or die "Can't read 'arch.txt': $!";
    $got = <$in>;
    close $in;
}
if ($got ne $arch) {
    if (-e "Errno.pm") {
        print "Removing old 'Errno.pm'\n";
        unlink "Errno.pm" 
            or die "Failed to remove out of date 'Errno.pm': $!";
    }
    open my $out, ">", "arch.txt" or die "Can't write 'arch.txt': $!";
    print $out $arch;
    close $out;
}

WriteMakefile(
    NAME	 => 'Errno',
    VERSION_FROM => 'Errno_pm.PL',
    PL_FILES	 => {'Errno_pm.PL'=>'Errno.pm'},
    PM		 => {'Errno.pm' => '$(INST_LIBDIR)/Errno.pm'},
    'clean'	 => {FILES => 'Errno.pm arch.txt'},
    'dist'	 => {
	COMPRESS => 'gzip -9f',
	SUFFIX   => '.gz', 
	DIST_DEFAULT => 'd/Errno.pm tardist',
    },
);

sub MY::postamble {
    my $TARG = MM->catfile('d','Errno.pm');
qq!$TARG : Makefile
	echo '#This is a dummy file so CPAN will find a VERSION' > $TARG
	echo 'package Errno;' >> $TARG
	echo '\$\$VERSION = "\$(VERSION)";' >>$TARG
	echo '#This is to make sure require will return an error' >>$TARG
	echo '0;' >>$TARG

!
}
