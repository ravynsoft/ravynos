package Locale::Maketext::Guts;

use Locale::Maketext;

our $VERSION = '1.20';

=head1 NAME

Locale::Maketext::Guts - Deprecated module to load Locale::Maketext utf8 code

=head1 SYNOPSIS

  # Do this instead please
  use Locale::Maketext

=head1 DESCRIPTION

Previously Local::Maketext::GutsLoader performed some magic to load
Locale::Maketext when utf8 was unavailable. The subs this module provided
were merged back into Locale::Maketext

=cut

1;
