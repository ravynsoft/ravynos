#! perl -w

use File::Spec;
my $perl;
BEGIN {
  $perl = File::Spec->rel2abs($^X);
  $perl = qq{"$perl"}; # So it doesn't fail when there are spaces.
}

use strict;
use Test::More;
BEGIN { 
  if ($^O eq 'VMS') {
    # So we can get the return value of system()
    require vmsish;
    import vmsish;
  }
}

plan tests => 7;

require_ok "ExtUtils::CBuilder";

my $b = eval { ExtUtils::CBuilder->new(quiet => 1) };
ok( $b, "got CBuilder object" ) or diag $@;

# test missing compiler
{

    my $b1 = ExtUtils::CBuilder->new(quiet => 1);

    configure_fake_missing_compilers($b1);

    # This will fork a child that will print
    #    'Can't exec "djaadjfkadjkfajdf"'
    # or similar on STDERR; so make sure fd2 is temporarily redirected to
    # oblivion before the fork
    open(OLDERR, ">&STDERR") or die "Can't dup STDERR: $!";
    open(STDERR, ">", File::Spec->devnull()) or die "Can't redirect STDERR: $!";
    my $res = $b1->have_compiler;
    open(STDERR, ">&OLDERR") or die "Can't restore STDERR: $!";
    close(OLDERR);

    is($res, 0, "have_compiler: fake missing cc" );
}
{
    my $b2 = ExtUtils::CBuilder->new(quiet => 1);
    configure_fake_missing_compilers($b2);

    open(OLDERR, ">&STDERR") or die "Can't dup STDERR: $!";
    open(STDERR, ">", File::Spec->devnull()) or die "Can't redirect STDERR: $!";
    my $res = $b2->have_cplusplus;
    open(STDERR, ">&OLDERR") or die "Can't restore STDERR: $!";
    close(OLDERR);

    is($res, 0, "have_cplusplus: fake missing c++" );
}

# test found compiler
{
my $b3 = ExtUtils::CBuilder->new(quiet => 1);
configure_fake_present_compilers($b3);
is( $b3->have_compiler, 1, "have_compiler: fake present cc" );
}
{
my $b4 = ExtUtils::CBuilder->new(quiet => 1);
configure_fake_present_compilers($b4);
is( $b4->have_cplusplus, 1, "have_cpp_compiler: fake present c++" );
}

# test missing cpp compiler

# test one non-exported subroutine
{
    my $type = ExtUtils::CBuilder::os_type();
    if ($type) {
        pass( "OS type $type located for $^O" );
    }
    else {
        pass( "OS type not yet listed for $^O" );
    }
}

sub configure_fake_missing_compilers {
    my $b = shift;
    my $bogus_path = 'djaadjfkadjkfajdf';
    $b->{config}{cc} = $bogus_path;
    $b->{config}{ld} = $bogus_path;
    $b->{have_cc} = undef;
    $b->{have_cxx} = undef;
}

sub configure_fake_present_compilers {
    my $b = shift;
    my $run_perl = "$perl -e1 --";
    $b->{config}{cc} = $run_perl;
    $b->{config}{ld} = $run_perl;
    $b->{config}{cxx} = $run_perl;
    $b->{have_cc} = undef;
    $b->{have_cxx} = undef;
}
