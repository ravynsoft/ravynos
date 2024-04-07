use strict;
use Test::More tests => 8;

# Grab all of the plain routines from File::Spec
use File::Spec;
use File::Spec::Win32;

require_ok($_) foreach qw(File::Spec File::Spec::Win32);


if ($^O eq 'VMS') {
    # hack:
    # Need to cause the %ENV to get populated or you only get the builtins at
    # first, and then something else can cause the hash to get populated.
    my %look_env = %ENV;
}
my $num_keys = keys %ENV;
File::Spec->tmpdir;
is scalar keys %ENV, $num_keys, "tmpdir() shouldn't change the contents of %ENV";

SKIP: {
    skip("Can't make list assignment to %ENV on this system", 1)
	if $^O eq 'VMS';

    local %ENV;
    File::Spec::Win32->tmpdir;
    is(scalar keys %ENV, 0, "Win32->tmpdir() shouldn't change the contents of %ENV");
}

File::Spec::Win32->tmpdir;
is(scalar keys %ENV, $num_keys, "Win32->tmpdir() shouldn't change the contents of %ENV");

# Changing tmpdir dynamically
for ('File::Spec', "File::Spec::Win32") {
  SKIP: {
    skip('sys$scratch: takes precedence over env on vms', 1)
	if $^O eq 'VMS';
    local $ENV{TMPDIR} = $_->catfile($_->curdir, 'lib');
    -d $ENV{TMPDIR} && -w _
       or skip "Can't create usable TMPDIR env var", 1;
    my $tmpdir1 = $_->tmpdir;
    $ENV{TMPDIR} = $_->catfile($_->curdir, 't');
    -d $ENV{TMPDIR} && -w _
       or skip "Can't create usable TMPDIR env var", 1;
    my $tmpdir2 = $_->tmpdir;
    isnt $tmpdir2, $tmpdir1, "$_->tmpdir works with changing env";
  }
}

ok(
    File::Spec->file_name_is_absolute(File::Spec->tmpdir()),
    "tmpdir() always returns an absolute path"
);
