#!./perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');

    eval { require AnyDBM_File }; # not all places have dbm* functions
    skip_all("No dbm functions") if $@;
}

plan tests => 5;

# This is [20020104.007 (#8179)] "coredump on dbmclose"

my $filename = tempfile();

my $prog = <<'EOC';
package Foo;
$filename = '@@@@';
sub new {
        my $proto = shift;
        my $class = ref($proto) || $proto;
        my $self  = {};
        bless($self,$class);
        my %LT;
        dbmopen(%LT, $filename, 0666) ||
	    die "Can't open $filename because of $!\n";
        $self->{'LT'} = \%LT;
        return $self;
}
sub DESTROY {
        my $self = shift;
	dbmclose(%{$self->{'LT'}});
	1 while unlink $filename;
	1 while unlink glob "$filename.*";
	print "ok\n";
}
package main;
$test = Foo->new(); # must be package var
EOC

$prog =~ s/\@\@\@\@/$filename/;

fresh_perl_is("require AnyDBM_File;\n$prog", 'ok', {}, 'explicit require');
fresh_perl_is($prog, 'ok', {}, 'implicit require');

$prog = <<'EOC';
@INC = ();
dbmopen(%LT, $filename, 0666);
1 while unlink $filename;
1 while unlink glob "$filename.*";
die "Failed to fail!";
EOC

fresh_perl_like($prog, qr/No dbm on this machine/, {},
		'implicit require fails');
fresh_perl_like('delete $::{"AnyDBM_File::"}; ' . $prog,
		qr/No dbm on this machine/, {},
		'implicit require and no stash fails');

{ # undef 3rd arg
    local $^W = 1;
    local $SIG{__WARN__} = sub { ++$w };
    # Files may get created as a side effect of dbmopen, so ensure cleanup.
    my $leaf = 'pleaseletthisfilenotexist';
    dbmopen(%truffe, $leaf, undef);
    is $w, 1, '1 warning from dbmopen with undef third arg';
    unlink $leaf
        if -e $leaf;
    1 while unlink glob "$leaf.*";
}
