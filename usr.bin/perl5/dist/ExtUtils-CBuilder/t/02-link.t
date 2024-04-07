#! perl -w

use strict;
use Test::More;
BEGIN {
  if ($^O eq 'VMS') {
    # So we can get the return value of system()
    require vmsish;
    import vmsish;
  }
}
use ExtUtils::CBuilder;
use File::Spec;

# TEST does not like extraneous output
my $quiet = $ENV{PERL_CORE} && !$ENV{HARNESS_ACTIVE};
my ($source_file, $object_file, $exe_file);

my $b = ExtUtils::CBuilder->new(quiet => $quiet);

# test plan
if ($^O =~ / ^ ( MSWin32 | os390 ) $ /xi) {
  plan skip_all => "link_executable() is not implemented yet on $^O";
}
elsif ( ! $b->have_compiler ) {
  plan skip_all => "no compiler available for testing";
}
else {
  plan tests => 8;
}

ok $b, "created EU::CB object";

$source_file = File::Spec->catfile('t', 'linkt.c');
{
  open my $FH, '>', $source_file or die "Can't create $source_file: $!";
  print $FH "int main(void) { return 11; }\n";
  close $FH;
}
ok -e $source_file, "generated '$source_file'";

# Compile
eval { $object_file = $b->compile(source => $source_file) };
is $@, q{}, "no exception from compilation";
ok -e $object_file, "found object file";

# Link
SKIP: {
  skip "error compiling source", 4
    unless -e $object_file;

  my @temps;
  eval { ($exe_file, @temps) = $b->link_executable(objects => $object_file) };
  is $@, q{}, "no exception from linking";
  ok -e $exe_file, "found executable file";
  ok -x $exe_file, "executable file appears to be executable";

  if ($^O eq 'os2') {		# Analogue of LDLOADPATH...
          # Actually, not needed now, since we do not link with the generated DLL
    my $old = OS2::extLibpath();	# [builtin function]
    $old = ";$old" if defined $old and length $old;
    # To pass the sanity check, components must have backslashes...
    OS2::extLibpath_set(".\\$old");
  }

  # Try the executable
  my $ec = my_system($exe_file);
  is( $ec, 11, "got expected exit code from executable" )
    or diag( $ec == -1 ? "Could not run '$exe_file': $!\n" 
                       : "Unexpected exit code '$ec'\n");
}

# Clean up
for ($source_file, $object_file, $exe_file) {
  tr/"'//d;
  1 while unlink;
}

if ($^O eq 'VMS') {
   1 while unlink 'LINKT.LIS';
   1 while unlink 'LINKT.OPT';
}

sub my_system {
  my $cmd = shift;
  my $ec;
  if ($^O eq 'VMS') {
    # Preserve non-posixified status and don't bit shift the result
    # because we're running under "use vmsish";
    $ec = system("mcr $cmd");
    return $ec;
  }
  else {
    $ec = system($cmd);
    return $ec == -1 ? -1 : $ec >> 8;
  }
}
