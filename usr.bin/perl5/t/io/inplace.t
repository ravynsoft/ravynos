#!./perl
use strict;
chdir 't' if -d 't';
require './test.pl';

$^I = $^O eq 'VMS' ? '_bak' : '.bak';

plan( tests => 8 );

my @tfiles     = (tempfile(), tempfile(), tempfile());
my @tfiles_bak = map "$_$^I", @tfiles;

END { unlink_all(@tfiles_bak); }

for my $file (@tfiles) {
    runperl( prog => 'print qq(foo\n);', 
             args => ['>', $file] );
}

@ARGV = @tfiles;

while (<>) {
    s/foo/bar/;
}
continue {
    print;
}

is ( runperl( prog => 'print<>;', args => \@tfiles ), 
     "bar\nbar\nbar\n", 
     "file contents properly replaced" );

is ( runperl( prog => 'print<>;', args => \@tfiles_bak ), 
     "foo\nfoo\nfoo\n", 
     "backup file contents stay the same" );

our @ifiles = ( tempfile(), tempfile(), tempfile() );

{
    for my $file (@ifiles) {
        runperl( prog => 'print qq(bar\n);',
                 args => [ '>', $file ] );
    }

    local $^I = '';
    local @ARGV = @ifiles;

    while (<>) {
        print "foo$_";
    }

    is(scalar(@ARGV), 0, "consumed ARGV");

    # runperl may quote its arguments, so don't expect to be able
    # to reuse things you send it.

    my @my_ifiles = @ifiles;
    is( runperl( prog => 'print<>;', args => \@my_ifiles ),
        "foobar\nfoobar\nfoobar\n",
        "normal inplace edit");
}

# test * equivalence RT #70802
{
    for my $file (@ifiles) {
        runperl( prog => 'print qq(bar\n);',
        args => [ '>', $file ] );
    }

    local $^I = '*';
    local @ARGV = @ifiles;

    while (<>) {
        print "foo$_";
    }

    is(scalar(@ARGV), 0, "consumed ARGV");

    my @my_ifiles = @ifiles;
    is( runperl( prog => 'print<>;', args => \@my_ifiles ),
        "foobar\nfoobar\nfoobar\n",
        "normal inplace edit");
}

END { unlink_all(@ifiles); }

{
    my @tests =
      ( # opts, code, result, name, $TODO
       [ "-n", "die", "bar\n", "die shouldn't touch file" ],
       [ "-n", "last", "", "last should update file" ],
      );
    our $file = tempfile() ;

    for my $test (@tests) {
        (my ($opts, $code, $result, $name), our $TODO) = @$test;
        open my $fh, ">", $file or die;
        print $fh "bar\n";
        close $fh;

        runperl( prog => $code,
                 switches => [ grep length, "-i", $opts ],
                 args => [ $file ],
                 stderr => 1, # discarded
               );
        open $fh, "<", $file or die;
        my $data = do { local $/; <$fh>; };
        close $fh;
        is($data, $result, $name);
    }
}
