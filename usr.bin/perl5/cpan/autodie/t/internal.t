#!/usr/bin/perl
use strict;

use Scalar::Util qw(blessed);

use constant NO_SUCH_FILE => "this_file_or_dir_had_better_not_exist_XYZZY";

use Test::More tests => 7;

use Fatal();

# Silence the warnings from using Fatal qw(:lexical)

# Lexical tests using the internal interface.

my @warnings;
eval {
    # Filter out deprecation warning (no warnings qw(deprecated) does
    # not seem to work for some reason)
    local $SIG{'__WARN__'} = sub {
        push(@warnings, @_) unless $_[0] =~ m/Fatal qw\(:lexical/;
    };
    Fatal->import(qw(:lexical :void))
};
like($@, qr{:void cannot be used with lexical}, ":void can't be used with :lexical");
warn($_) while shift @warnings;

eval { Fatal->import(qw(open close :lexical)) };
like($@, qr{:lexical must be used as first}, ":lexical must come first");

{
	BEGIN {
	    # Filter out deprecation warning (no warnings qw(deprecated) does
	    # not seem to work for some reason)
	    local $SIG{'__WARN__'} = sub {
	        push(@warnings, @_) unless $_[0] =~ m/Fatal qw\(:lexical/;
	    };
	    import Fatal qw(:lexical chdir);
	};
	warn($_) while shift @warnings;
	eval { chdir(NO_SUCH_FILE); };
	my $err = $@;
	like ($err, qr/^Can't chdir/, "Lexical fatal chdir");
	{
		no Fatal qw(:lexical chdir);
		eval { chdir(NO_SUCH_FILE); };
		is ($@, "", "No lexical fatal chdir");
        }

	eval { chdir(NO_SUCH_FILE); };
	$err = $@;
	like ($err, qr/^Can't chdir/, "Lexical fatal chdir returns");
}

eval { chdir(NO_SUCH_FILE); };
is($@, "", "Lexical chdir becomes non-fatal out of scope.");

eval { Fatal->import('2+2'); };
like($@,qr{Bad subroutine name},"Can't use fatal with invalid sub names");
