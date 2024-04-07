#!./perl -w

BEGIN {
  if ($ENV{PERL_CORE}) {
    chdir 't' if -d 't';
    #@INC = '../lib';
  }
}

use strict;
use File::Spec;
use File::Path;

my $dir;
BEGIN
{
    $dir = File::Spec->catdir( "auto-$$" );
    unshift @INC, $dir;
}

use Test::More tests => 21;

sub write_file {
    my ($file, $text) = @_;
    open my $fh, '>', $file
      or die "Could not open file '$file' for writing: $!";
    print $fh $text;
    close $fh;
}

# First we must set up some autoloader files
my $fulldir = File::Spec->catdir( $dir, 'auto', 'Foo' );
mkpath( $fulldir ) or die "Can't mkdir '$fulldir': $!";

write_file( File::Spec->catfile( $fulldir, 'foo.al' ), <<'EOT' );
package Foo;
sub foo { shift; shift || "foo" }
1;
EOT

write_file( File::Spec->catfile( $fulldir, 'bazmarkhian.al' ), <<'EOT' );
package Foo;
sub bazmarkhianish { shift; shift || "baz" }
1;
EOT

my $blechanawilla_text = <<'EOT';
package Foo;
sub blechanawilla { compilation error (
EOT
write_file( File::Spec->catfile( $fulldir, 'blechanawilla.al' ), $blechanawilla_text );
# This is just to keep the old SVR3 systems happy; they may fail
# to find the above file so we duplicate it where they should find it.
write_file( File::Spec->catfile( $fulldir, 'blechanawil.al' ), $blechanawilla_text );

write_file( File::Spec->catfile( $fulldir, 'notreached.al' ), <<'EOT' );
package Foo;
sub notreached { die "Should not be reached!" }
1;
EOT

# Let's define the package
package Foo;
require AutoLoader;
AutoLoader->import( 'AUTOLOAD' );

sub new { bless {}, shift };
sub foo;
sub bazmarkhianish; 
sub notreached;

package main;

my $foo = Foo->new();

my $result = $foo->can( 'foo' );
ok( $result,               'can() first time' );
is( $foo->foo, 'foo', 'autoloaded first time' );
is( $foo->foo, 'foo', 'regular call' );
is( $result,   \&Foo::foo, 'can() returns ref to regular installed sub' );

eval {
    $foo->will_fail;
};
like( $@, qr/^Can't locate/, 'undefined method' );

$result = $foo->can( 'will_fail' );
ok( ! $result,               'can() should fail on undefined methods' );

# Used to be trouble with this
eval {
    my $foo = Foo->new();
    die "oops";
};
like( $@, qr/oops/, 'indirect method call' );

# Pass regular expression variable to autoloaded function.  This used
# to go wrong because AutoLoader used regular expressions to generate
# autoloaded filename.
'foo' =~ /(\w+)/;

is( $foo->bazmarkhianish($1), 'foo', 'autoloaded method should not stomp match vars' );
is( $foo->bazmarkhianish($1), 'foo', '(again)' );

# Used to retry long subnames with shorter filenames on any old
# exception, including compilation error.  Now AutoLoader only
# tries shorter filenames if it can't find the long one.
eval {
  $foo->blechanawilla;
};
like( $@, qr/syntax error/i, 'require error propagates' );

# test recursive autoloads
write_file( File::Spec->catfile( $fulldir, 'a.al' ), <<'EOT' );
package Foo;
BEGIN { b() }
sub a { ::ok( 1, 'adding a new autoloaded method' ); }
1;
EOT
write_file( File::Spec->catfile( $fulldir, 'b.al' ), <<'EOT' );
package Foo;
sub b { ::ok( 1, 'adding a new autoloaded method' ) }
1;
EOT

Foo::a();

# Test whether autoload_sub works without actually executing the function
ok(!defined(&Foo::notreached), "Foo::notreached unknown to boot");
AutoLoader::autoload_sub("Foo::notreached");
ok(defined(&Foo::notreached), "Foo::notreached loaded by autoload_sub");

# Make sure that repeatedly calling autoload_sub is not a problem:
AutoLoader::autoload_sub("Foo::notreached");
eval {Foo::notreached;};
ok($@ && $@ =~ /Should not/, "Foo::notreached works as expected");

package Bar;
AutoLoader->import();
::ok( ! defined &AUTOLOAD, 'AutoLoader should not export AUTOLOAD by default' );
::ok( ! defined &can,      '... nor can()' );

package Foo;
AutoLoader->unimport();
eval { Foo->baz() };
::like( $@, qr/locate object method "baz"/,
        'unimport() should remove imported AUTOLOAD()' );

package Baz;

sub AUTOLOAD { 'i am here' }

AutoLoader->import();
AutoLoader->unimport();

::is( Baz->AUTOLOAD(), 'i am here', '... but not non-imported AUTOLOAD()' );


package SomeClass;
use AutoLoader 'AUTOLOAD';
sub new {
    bless {} => shift;
}

package main;

$INC{"SomeClass.pm"} = $0; # Prepare possible recursion
{
    my $p = SomeClass->new();
} # <-- deep recursion in AUTOLOAD looking for SomeClass::DESTROY?
::ok(1, "AutoLoader shouldn't loop forever if \%INC is modified");

# Now test the bug that lead to AutoLoader 0.67:
# If the module is loaded from a file name different than normal,
# we could formerly have trouble finding autosplit.ix
# Contributed by Christoph Lamprecht.
# Recreate the following file structure:
# auto/MyAddon/autosplit.ix
# auto/MyAddon/testsub.al
# MyModule.pm
SCOPE: {
    my $autopath = File::Spec->catdir( $dir, 'auto', 'MyAddon' );
    mkpath( $autopath ) or die "Can't mkdir '$autopath': $!";
    my $autosplit_text = <<'EOT';
# Index created by AutoSplit for MyModule.pm
#    (file acts as timestamp)
package MyAddon;
sub testsub ;
1;
EOT
    write_file( File::Spec->catfile( $autopath, 'autosplit.ix' ), $autosplit_text );

    my $testsub_text = <<'EOT';
# NOTE: Derived from MyModule.pm.
# Changes made here will be lost when autosplit is run again.
# See AutoSplit.pm.
package MyAddon;

#line 13 "MyModule.pm (autosplit into auto/MyAddon/testsub.al)"
sub testsub{
    return "MyAddon";
}

1;
# end of MyAddon::testsub
EOT
    write_file( File::Spec->catfile( $autopath, 'testsub.al' ), $testsub_text);

    my $mymodule_text = <<'EOT';
use strict;
use warnings;
package MyModule;
sub testsub{return 'MyModule';}

package MyAddon;
our @ISA = ('MyModule');
BEGIN{$INC{'MyAddon.pm'} = __FILE__}
use AutoLoader 'AUTOLOAD';
1;
__END__

sub testsub{
    return "MyAddon";
}
EOT
    write_file( File::Spec->catfile( $dir, 'MyModule.pm' ), $mymodule_text);

    require MyModule;

    my $res = MyAddon->testsub();
    ::is ($res , 'MyAddon', 'invoke MyAddon::testsub');
}

# cleanup
END {
    return unless $dir && -d $dir;
    rmtree $dir;
}

