#!perl -w

# Can't use Test::Simple/More, they depend on Exporter.
my $test;
sub ok ($;$) {
    my($ok, $name) = @_;

    # You have to do it this way or VMS will get confused.
    printf "%sok %d%s\n", ($ok ? '' : 'not '), $test,
      (defined $name ? " - $name" : '');

    printf "# Failed test at line %d\n", (caller)[2] unless $ok;

    $test++;
    return $ok;
}


BEGIN {
    $test = 1;
    print "1..2\n";
    require Exporter;
    ok( 1, 'Exporter compiled' );
}

package Foo;
Exporter->import("import");
our @EXPORT_OK = qw/bar/;


package main;

{ # [perl #39739] Exporter::Heavy ignores custom $SIG{__WARN__} handlers
    my @warn;

    local $SIG{__WARN__} = sub { push @warn, join "", @_ };
    eval { Foo->import(":quux") };
    ok(grep(/"quux" is not defined/, @warn), "warnings captured");
}

