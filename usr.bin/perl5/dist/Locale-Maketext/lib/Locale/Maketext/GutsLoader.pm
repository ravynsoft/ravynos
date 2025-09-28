package Locale::Maketext::GutsLoader;

use Locale::Maketext;

our $VERSION = '1.20';

sub zorp { return scalar @_ }

=head1 NAME

Locale::Maketext::GutsLoader - Deprecated module to load Locale::Maketext utf8 code

=head1 SYNOPSIS

  # Do this instead please
  use Locale::Maketext

=head1 DESCRIPTION

Previously Locale::Maketext::Guts performed some magic to load
Locale::Maketext when utf8 was unavailable. The subs this module provided
were merged back into Locale::Maketext.

=cut

1;
