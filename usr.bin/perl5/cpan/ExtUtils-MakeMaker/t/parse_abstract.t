#!/usr/bin/perl

use strict;
use warnings;

use lib 't/lib';
use ExtUtils::MakeMaker;
use File::Temp qw[tempfile];
use Test::More 'no_plan';

sub test_abstract {
    my($code, $package, $want, $name) = @_;

    local $Test::Builder::Level = $Test::Builder::Level + 1;

    my $ok = 0;
    for my $crlf (0, 1) {
        my ($fh,$file) = tempfile( DIR => 't', UNLINK => 1 );
        binmode $fh, $crlf ? ':crlf' : ':raw';
        print $fh $code;
        close $fh;
        # Hack up a minimal MakeMaker object.
        my $mm = bless { DISTNAME => $package }, "MM";
        my $have = $mm->parse_abstract($file);
        $ok += is( $have, $want, "$name :crlf=$crlf" ) ? 1 : 0;
    }
    return $ok;
}


test_abstract(<<END, "Foo", "Stuff and things", "Simple abstract");
=head1 NAME

Foo - Stuff and things
END


test_abstract(<<END, "NEXT", "Provide a pseudo-class NEXT (et al) that allows method redispatch", "Name.pm");
=head1 NAME

NEXT.pm - Provide a pseudo-class NEXT (et al) that allows method redispatch
END


test_abstract(<<END, "Compress::Raw::Zlib::FAQ", "Frequently Asked Questions about Compress::Raw::Zlib", "double dash");
=pod

Compress::Raw::Zlib::FAQ -- Frequently Asked Questions about Compress::Raw::Zlib
END


test_abstract(<<END, "Foo", "This is", "Only in POD");
# =pod

Foo - This is not in pod

=cut

Foo - This isn't in pod either

=pod

Foo - This is

Foo - So is this.
END


test_abstract(<<END, "Foo", "the abstract", "more spaces");
=pod

Foo   -  the abstract
END

test_abstract(<<END, "Catalyst::Plugin::Authentication", "Infrastructure plugin for the Catalyst authentication framework.", "contains a line break");
=pod

=head1 NAME

Catalyst::Plugin::Authentication - Infrastructure plugin for the Catalyst
authentication framework.
END
