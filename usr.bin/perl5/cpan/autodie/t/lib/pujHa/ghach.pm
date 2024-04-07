package pujHa::ghach;

# Translator notes: reH Hegh is Kligon for "always dying".
# It was the original name for this testing pragma, but
# it lacked an apostrophe, which better shows how Perl is
# useful in Klingon naming schemes.

# The new name is pujHa'ghach is "thing which is not weak".
#   puj   -> be weak (verb)
#   -Ha'  -> not
#   ghach -> normalise -Ha' verb into noun.
#
# I'm not use if -wI' should be used here.  pujwI' is "thing which
# is weak".  One could conceivably use "pujHa'wI'" for "thing which
# is not weak".

use strict;
use warnings;

use parent qw(autodie);

sub exception_class {
    return "pujHa::ghach::Dotlh";      # Dotlh - status
}

1;
