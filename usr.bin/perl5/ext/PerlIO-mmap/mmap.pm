package PerlIO::mmap;
use strict;
use warnings;
our $VERSION = '0.017';

use XSLoader;
XSLoader::load(__PACKAGE__, __PACKAGE__->VERSION);

1;

__END__

=head1 NAME

PerlIO::mmap - Memory mapped IO

=head1 SYNOPSIS

 open my $fh, '<:mmap', $filename;

=head1 DESCRIPTION

This layer does C<read> and C<write> operations by mmap()ing the file if possible, but falls back to the default behavior if not.

=head1 IMPLEMENTATION NOTE

C<PerlIO::mmap> only exists to use XSLoader to load C code that provides support for using memory mapped IO. One does not need to explicitly C<use PerlIO::mmap;>.

=cut

