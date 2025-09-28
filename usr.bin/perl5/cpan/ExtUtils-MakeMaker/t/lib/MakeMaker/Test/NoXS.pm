package MakeMaker::Test::NoXS;

# Disable all XS loading.

use strict;
use warnings;
use Carp;

require DynaLoader;
require XSLoader;

# Things like Cwd key on this to decide if they're running miniperl
delete $DynaLoader::{boot_DynaLoader};

if ($^O eq 'MSWin32') {
    # Load Win32. Then clear the stash of all other entries but GetCwd and SetChildShowWindow
    # SetChildShowWindow and GetCwd are provided by perl core in modern perls, so we
    # can use them in miniperl. on older perls, we can load them from Win32 so the
    # test can proceed as normal.

    require Win32;

    foreach my $slot (keys %Win32::) {
        next if $slot eq 'GetCwd';
        next if $slot eq 'SetChildShowWindow';
        delete $Win32::{$slot};
    }
}

# This isn't 100%.  Things like Win32.pm will crap out rather than
# just not load.  See ExtUtils::MM->_is_win95 for an example
no warnings 'redefine';
*DynaLoader::bootstrap = sub { confess "Tried to load XS for @_"; };
*XSLoader::load        = sub { confess "Tried to load XS for @_"; };

1;
