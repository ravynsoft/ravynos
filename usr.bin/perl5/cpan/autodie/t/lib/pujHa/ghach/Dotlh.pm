package pujHa::ghach::Dotlh;

# Translator notes: Dotlh = status

# Ideally this should be le'wI' - Thing that is exceptional. ;)
# Unfortunately that results in a file called .pm, which may cause
# problems on some filesystems.

use strict;
use warnings;

use parent qw(autodie::exception);

sub stringify {
    my ($this) = @_;

    my $error = $this->SUPER::stringify;

    return "QaghHommeyHeylIjmo':\n" .   # Due to your apparent minor errors
           "$error\n" .
           "lujqu'";                    # Epic fail


}

1;

__END__

# The following was a really neat idea, but currently autodie
# always pushes values in $! to format them, which loses the
# Klingon translation.

use Errno qw(:POSIX);
use Scalar::Util qw(dualvar);

my %translation_for = (
    EPERM()  => q{Dachaw'be'},        # You do not have permission
    ENOENT() => q{De' vItu'laHbe'},   # I cannot find this information.
);

sub errno {
    my ($this) = @_;

    my $errno = int $this->SUPER::errno;

    warn "In tlhIngan errno - $errno\n";

    if ( my $tlhIngan = $translation_for{ $errno } ) {
        return dualvar( $errno, $tlhIngan );
    }

    return $!;

}

1;


