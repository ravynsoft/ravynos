#!perl -w

# Check that modifying %INC during an @INC hook does not
# clobber the hook by modifying @INC at the same time.
# See GitHub Issue #20577

chdir "t" if -d "t";
require './test.pl';
skip_all_if_miniperl("as PerlIO layer 'scalar' not supported under miniperl");
set_up_inc( '../lib' );
eval <<'EOF' or die $@;
{
    my %fatpacked;

    $fatpacked{"Test1.pm"} = <<'TEST1';
  package Test1;
  sub import {
      my $filename = 'Test2.pm';
      $INC{$filename} = "the_test_file";
  }
  1;
TEST1

    $fatpacked{"Test2.pm"} = <<'TEST2';
  package Test2;
  use Test1;
  1;
TEST2

    my $class = 'FatPacked';
    no strict 'refs';

    *{"${class}::INC"} = sub {
        if ( my $fat = $_[0]{ $_[1] } ) {
            open my $fh, '<', \$fat
              or die;
            return $fh;
        }
        return;
    };

    unshift @INC, bless \%fatpacked, $class;
}
1
EOF

ok(UNIVERSAL::isa($INC[0],"FatPacked"), '$INC[0] starts FatPacked');
ok(!exists $INC{"Test1.pm"}, 'Test1.pm not in %INC');
ok(!exists $INC{"Test2.pm"}, 'Test2.pm not in %INC');
my $ok= eval "use Test2; 1";
my $err= !$ok ? $@ : undef;
is($err,undef,"No error loading Test2");
is($ok,1,"Loaded Test2 successfully");
ok(UNIVERSAL::isa($INC[0],"FatPacked"), '$INC[0] is still FatPacked');
ok(UNIVERSAL::isa($INC{"Test1.pm"},"FatPacked"), '$INC{"Test1.pm"} is still FatPacked');
is($INC{"Test2.pm"},"the_test_file", '$INC{"Test2.pm"} is as expected');
is($INC[0],$INC{"Test1.pm"},'Same object in @INC and %INC');
done_testing();
