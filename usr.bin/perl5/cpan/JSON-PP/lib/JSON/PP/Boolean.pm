package JSON::PP::Boolean;

use strict;
use warnings;
use overload ();
overload::unimport('overload', qw(0+ ++ -- fallback));
overload::import('overload',
    "0+"     => sub { ${$_[0]} },
    "++"     => sub { $_[0] = ${$_[0]} + 1 },
    "--"     => sub { $_[0] = ${$_[0]} - 1 },
    fallback => 1,
);

our $VERSION = '4.16';

1;

__END__

=head1 NAME

JSON::PP::Boolean - dummy module providing JSON::PP::Boolean

=head1 SYNOPSIS

 # do not "use" yourself

=head1 DESCRIPTION

This module exists only to provide overload resolution for Storable and similar modules. See
L<JSON::PP> for more info about this class.

=head1 AUTHOR

This idea is from L<JSON::XS::Boolean> written by Marc Lehmann <schmorp[at]schmorp.de>

=head1 LICENSE

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself.

=cut

