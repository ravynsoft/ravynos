#!/usr/bin/perl -w

# Test that open still honors the open pragma.

use strict;
use warnings;

use autodie;

use Fcntl;
use File::Temp;
use Test::More;

if( $] < '5.01000' ) {
    plan skip_all => "autodie does not honor the open pragma before 5.10";
}
else {
    plan "no_plan";
}

# Test with an open pragma on
{
    use open IN => ':encoding(utf8)', OUT => ':utf8';

    # Test the standard handles and all newly opened handles are utf8
    my $file = File::Temp->new;
    my $txt = "autodie is MËTÁŁ";

    # open for writing
    {
        open my $fh, ">", $file;

        my @layers = PerlIO::get_layers($fh);
        ok( (grep { $_ eq 'utf8' } @layers), "open write honors open pragma" ) or diag join ", ", @layers;

        print $fh $txt;
        close $fh;
    }

    # open for reading, explicit
    {
        open my $fh, "<", $file;

        my @layers = PerlIO::get_layers($fh);
        ok( (grep { $_ eq 'encoding(utf8)' } @layers), "open read honors open pragma" ) or diag join ", ", @layers;

        is join("\n", <$fh>), $txt;
    }

    # open for reading, implicit
    {
        open my($fh), $file;

        my @layers = PerlIO::get_layers($fh);
        ok( (grep { $_ eq 'encoding(utf8)' } @layers), "open implicit read honors open pragma" ) or diag join ", ", @layers;

        is join("\n", <$fh>), $txt;
    }

    # open for read/write
    {
        open my $fh, "+>", $file;

        my @layers = PerlIO::get_layers($fh);
        ok( (grep { $_ eq 'utf8' } @layers), "open implicit read honors open pragma" ) or diag join ", ", @layers;
    }

    # open for append
    {
        open my $fh, ">>", $file;

        my @layers = PerlIO::get_layers($fh);
        ok( (grep { $_ eq 'utf8' } @layers), "open implicit read honors open pragma" ) or diag join ", ", @layers;
    }

    # raw
    {
        open my $fh, ">:raw", $file;

        my @layers = PerlIO::get_layers($fh);

        ok( !(grep { $_ eq 'utf8' } @layers), 'open pragma is not used if raw is specified' ) or diag join ", ", @layers;
    }
}


# Test without open pragma
{
    my $file = File::Temp->new;
    open my $fh, ">", $file;

    my @layers = PerlIO::get_layers($fh);
    ok( grep(!/utf8/, @layers), "open pragma remains lexical" ) or diag join ", ", @layers;
}


# sysopen
{
    use open IN => ':encoding(utf8)', OUT => ':utf8';

    # Test the standard handles and all newly opened handles are utf8
    my $file = File::Temp->new;
    my $txt = "autodie is MËTÁŁ";

    # open for writing only
    {
        sysopen my $fh, $file, O_CREAT|O_TRUNC|O_WRONLY;

        my @layers = PerlIO::get_layers($fh);
        ok( (grep { $_ eq 'utf8' } @layers), "open write honors open pragma" ) or diag join ", ", @layers;

        print $fh $txt;
        close $fh;
    }

    # open for reading only
    {
        sysopen my $fh, $file, O_RDONLY;

        my @layers = PerlIO::get_layers($fh);
        ok( (grep { $_ eq 'encoding(utf8)' } @layers), "open read honors open pragma" ) or diag join ", ", @layers;

        is join("\n", <$fh>), $txt;
    }

    # open for reading and writing
    {
        sysopen my $fh, $file, O_RDWR;

        my @layers = PerlIO::get_layers($fh);
        ok( (grep { $_ eq 'utf8' } @layers), "open read/write honors open write pragma" ) or diag join ", ", @layers;

        is join("\n", <$fh>), $txt;
    }
}
