#!./perl -- # Perl Rules

BEGIN {
    if ($^O eq 'VMS') {
        print "1..0 # Skip: needs porting, perhaps imitating Win32 mechanisms\n";
	exit 0;
    }
    require "../../t/test.pl";
}
use strict;
use warnings;

plan 3;

  my $file = 't/fd.t';

# [perl #76474]
{
  my $stderr = runperl(
     switches => ['-MIPC::Open3', '-w'],
     prog => "open STDIN, q _${file}_ or die \$!; open3(q _<&0_, my \$out, undef, \$ENV{PERLEXE}, q _-e0_)",
     stderr => 1,
  );

  is $stderr, '',
   "dup STDOUT in a child process by using its file descriptor";
}

{
  my $want = qr{\A#!\./perl -- # Perl Rules\r?\z};
  open my $fh, '<', $file or die "Can't open $file: $!";
  my $have = <$fh>;
  chomp $have;
  like($have, $want, 'We can find our test string');
  close $fh;

  fresh_perl_like(<<"EOP",
use IPC::Open3;
open FOO, '<', '$file' or die \$!;
open3('<&' . fileno FOO, my \$out, undef, \$ENV{PERLEXE}, '-eprint scalar <STDIN>');
print <\$out>;
EOP
		  $want,
		  undef,
		  'Numeric file handles are duplicated correctly'
      );
}
