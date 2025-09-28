#!./perl -w
# t/sortkeys.t - Test Sortkeys()

use strict;
use warnings;

use Data::Dumper;
use Test::More tests => 22;
use lib qw( ./t/lib );
use Testing qw( _dumptostr );

run_tests_for_sortkeys();
SKIP: {
    skip "XS version was unavailable, so we already ran with pure Perl", 13 
        if $Data::Dumper::Useperl;
    local $Data::Dumper::Useperl = 1;
    run_tests_for_sortkeys();
}

sub run_tests_for_sortkeys {
    note("\$Data::Dumper::Useperl = $Data::Dumper::Useperl");

    my %d = (
        delta   => 'd',
        beta    => 'b',
        gamma   => 'c',
        alpha   => 'a',
    );
    
    {
        my ($obj, %dumps, $sortkeys, $starting);
    
        note("\$Data::Dumper::Sortkeys and Sortkeys() set to true value");
    
        $starting = $Data::Dumper::Sortkeys;
        $sortkeys = 1;
        local $Data::Dumper::Sortkeys = $sortkeys;
        $obj = Data::Dumper->new( [ \%d ] );
        $dumps{'ddskone'} = _dumptostr($obj);
        local $Data::Dumper::Sortkeys = $starting;
    
        $obj = Data::Dumper->new( [ \%d ] );
        $obj->Sortkeys($sortkeys);
        $dumps{'objskone'} = _dumptostr($obj);
    
        is($dumps{'ddskone'}, $dumps{'objskone'},
            "\$Data::Dumper::Sortkeys = 1 and Sortkeys(1) are equivalent");
        like($dumps{'ddskone'},
            qr/alpha.*?beta.*?delta.*?gamma/s,
            "Sortkeys returned hash keys in Perl's default sort order");
        %dumps = ();
    
    }
    
    {
        my ($obj, %dumps, $starting);
    
        note("\$Data::Dumper::Sortkeys and Sortkeys() set to coderef");
    
        $starting = $Data::Dumper::Sortkeys;
        local $Data::Dumper::Sortkeys = \&reversekeys;
        $obj = Data::Dumper->new( [ \%d ] );
        $dumps{'ddsksub'} = _dumptostr($obj);
        local $Data::Dumper::Sortkeys = $starting;
    
        $obj = Data::Dumper->new( [ \%d ] );
        $obj->Sortkeys(\&reversekeys);
        $dumps{'objsksub'} = _dumptostr($obj);
    
        is($dumps{'ddsksub'}, $dumps{'objsksub'},
            "\$Data::Dumper::Sortkeys = CODEREF and Sortkeys(CODEREF) are equivalent");
        like($dumps{'ddsksub'},
            qr/gamma.*?delta.*?beta.*?alpha/s,
            "Sortkeys returned hash keys per sorting subroutine");
        %dumps = ();
    
    }
    
    {
        my ($obj, %dumps, $starting);
    
        note("\$Data::Dumper::Sortkeys and Sortkeys() set to coderef with filter");
        $starting = $Data::Dumper::Sortkeys;
        local $Data::Dumper::Sortkeys = \&reversekeystrim;
        $obj = Data::Dumper->new( [ \%d ] );
        $dumps{'ddsksub'} = _dumptostr($obj);
        local $Data::Dumper::Sortkeys = $starting;
    
        $obj = Data::Dumper->new( [ \%d ] );
        $obj->Sortkeys(\&reversekeystrim);
        $dumps{'objsksub'} = _dumptostr($obj);
    
        is($dumps{'ddsksub'}, $dumps{'objsksub'},
            "\$Data::Dumper::Sortkeys = CODEREF and Sortkeys(CODEREF) select same keys");
        like($dumps{'ddsksub'},
            qr/gamma.*?delta.*?beta/s,
            "Sortkeys returned hash keys per sorting subroutine");
        unlike($dumps{'ddsksub'},
            qr/alpha/s,
            "Sortkeys filtered out one key per request");
        %dumps = ();
    
    }
    
    {
        my ($obj, %dumps, $sortkeys, $starting);
    
        note("\$Data::Dumper::Sortkeys(undef) and Sortkeys(undef)");
    
        $starting = $Data::Dumper::Sortkeys;
        $sortkeys = 0;
        local $Data::Dumper::Sortkeys = $sortkeys;
        $obj = Data::Dumper->new( [ \%d ] );
        $dumps{'ddskzero'} = _dumptostr($obj);
        local $Data::Dumper::Sortkeys = $starting;
    
        $obj = Data::Dumper->new( [ \%d ] );
        $obj->Sortkeys($sortkeys);
        $dumps{'objskzero'} = _dumptostr($obj);
    
        $sortkeys = undef;
        local $Data::Dumper::Sortkeys = $sortkeys;
        $obj = Data::Dumper->new( [ \%d ] );
        $dumps{'ddskundef'} = _dumptostr($obj);
        local $Data::Dumper::Sortkeys = $starting;
    
        $obj = Data::Dumper->new( [ \%d ] );
        $obj->Sortkeys($sortkeys);
        $dumps{'objskundef'} = _dumptostr($obj);
    
        is($dumps{'ddskzero'}, $dumps{'objskzero'},
            "\$Data::Dumper::Sortkeys = 0 and Sortkeys(0) are equivalent");
        is($dumps{'ddskzero'}, $dumps{'ddskundef'},
            "\$Data::Dumper::Sortkeys = 0 and = undef equivalent");
        is($dumps{'objkzero'}, $dumps{'objkundef'},
            "Sortkeys(0) and Sortkeys(undef) are equivalent");
        %dumps = ();
    
    }
    
    {
        my $warning = '';
        local $SIG{__WARN__} = sub { $warning = $_[0] };
    
        my ($obj, %dumps, $starting);
    
        note("\$Data::Dumper::Sortkeys and Sortkeys() set to coderef");
    
        $starting = $Data::Dumper::Sortkeys;
        local $Data::Dumper::Sortkeys = \&badreturnvalue;
        $obj = Data::Dumper->new( [ \%d ] );
        $dumps{'ddsksub'} = _dumptostr($obj);
        like($warning, qr/^Sortkeys subroutine did not return ARRAYREF/,
            "Got expected warning: sorting routine did not return array ref");
    }

}

sub reversekeys { return [ reverse sort keys %{+shift} ]; }

sub reversekeystrim {
    my $hr = shift;
    my @keys = sort keys %{$hr};
    shift(@keys);
    return [ reverse @keys ];
}

sub badreturnvalue { return { %{+shift} }; }
