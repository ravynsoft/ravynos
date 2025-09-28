package ExtUtils::ParseXS::Constants;
use strict;
use warnings;
use Symbol;

our $VERSION = '3.51';

=head1 NAME

ExtUtils::ParseXS::Constants - Initialization values for some globals

=head1 SYNOPSIS

  use ExtUtils::ParseXS::Constants ();

  $PrototypeRegexp = $ExtUtils::ParseXS::Constants::PrototypeRegexp;

=head1 DESCRIPTION

Initialization of certain non-subroutine variables in ExtUtils::ParseXS and some of its
supporting packages has been moved into this package so that those values can
be defined exactly once and then re-used in any package.

Nothing is exported.  Use fully qualified variable names.

=cut

# FIXME: THESE ARE NOT CONSTANTS!
our @InitFileCode;

# Note that to reduce maintenance, $PrototypeRegexp is used
# by ExtUtils::Typemaps, too!
our $PrototypeRegexp = "[" . quotemeta('\$%&*@;[]_') . "]";
our @XSKeywords      = qw( 
  REQUIRE BOOT CASE PREINIT INPUT INIT CODE PPCODE
  OUTPUT CLEANUP ALIAS ATTRS PROTOTYPES PROTOTYPE
  VERSIONCHECK INCLUDE INCLUDE_COMMAND SCOPE INTERFACE
  INTERFACE_MACRO C_ARGS POSTCALL OVERLOAD FALLBACK
  EXPORT_XSUB_SYMBOLS
);

our $XSKeywordsAlternation = join('|', @XSKeywords);

1;
