# Testing Pod::Simple::JustPod against *.pod in /t
use strict;
use warnings;

BEGIN {
  if($ENV{PERL_CORE}) {
    chdir 't';
    @INC = '../lib';
  }

  use Config;
  if ($Config::Config{'extensions'} !~ /\bEncode\b/) {
    print "1..0 # Skip: Encode was not built\n";
    exit 0;
  }
}

use File::Find;
use File::Spec;
use Test::More;

use Pod::Simple::JustPod;

my @test_files;

BEGIN {
  sub source_path {
    my $file = shift;
    if ($ENV{PERL_CORE}) {
      require File::Spec;
      my $updir = File::Spec->updir;
      my $dir   = File::Spec->catdir($updir, 'lib', 'Pod', 'Simple', 't');
      return File::Spec->catdir($dir, $file);
    }
    else {
      return $file;
    }
  }

  my @test_dirs = (
    File::Spec->catdir( source_path('t') ) ,
    File::Spec->catdir( File::Spec->updir, 't') ,
  );

  my $test_dir;
  foreach( @test_dirs ) {
    $test_dir = $_ and last if -e;
  }

  die "Can't find the test dir" unless $test_dir;
  print "# TESTDIR: $test_dir\n";

  sub wanted {
    push @test_files, $File::Find::name
      if $File::Find::name =~ /\.pod$/;
  }
  find(\&wanted , $test_dir );

  plan tests => scalar @test_files;
}

@test_files = sort @test_files;

my @skip_on_windows = qw{
  t/corpus/8859_7.pod
  t/corpus/laozi38p.pod
  t/junk2.pod
  t/perlcyg.pod
  t/perlfaq.pod
  t/perlvar.pod
  t/search60/A/x.pod
  t/search60/B/X.pod
  t/testlib1/hinkhonk/Glunk.pod
  t/testlib1/pod/perlflif.pod
  t/testlib1/pod/perlthng.pod
  t/testlib1/squaa/Glunk.pod
  t/testlib1/zikzik.pod
  t/testlib2/hinkhonk/Glunk.pod
  t/testlib2/pod/perlthng.pod
  t/testlib2/pod/perlzuk.pod
  t/testlib2/pods/perlzoned.pod
  t/testlib2/squaa/Wowo.pod
};

my $is_windows = $^O eq 'MSWin32';
foreach my $file (@test_files) {
  SKIP: {
    if ( $is_windows && grep { $_ eq $file } @skip_on_windows ) {
      skip "$file needs investigation on windows", 1;  
    }    

    my $parser = Pod::Simple::JustPod->new();
    $parser->complain_stderr(0);

    my $input;
    open( IN , '<:raw' , $file ) or die "$file: $!";
    $input .= $_ while (<IN>);
    close( IN );

    my $output;
    $parser->output_string( \$output );
    $parser->parse_string_document( $input );

    if ($parser->any_errata_seen()) {
      pass("Skip '$file' because of pod errors");
      next if "$]" lt '5.010.001';     # note() not found in earlier versions
      my $errata = $parser->errata_seen();
      foreach my $line_number (sort { $a <=> $b } keys %$errata) {
          foreach my $err_msg (sort @{$errata->{$line_number}}) {
              note("$file: $line_number: $err_msg");
          }
      }
      next;
    }

    my $encoding = $parser->encoding();
    if (defined $encoding) {
      eval { require Encode; };
      $input = Encode::decode($parser->encoding(), $input);
    }

    my @input = split "\n", $input;
    my $stripped_input = "";
    while (defined ($_ = shift @input)) {
      if (/ ^ = [a-z]+ /x) {
        my $line = "$_\n";

        if ($stripped_input eq "" || $_ !~ /^=pod/) {
          $stripped_input .= $line;
        }
        while (defined ($_ = shift @input)) {
          $stripped_input .= "$_\n";
          last if / ^ =cut /x;
        }
      }
    }
    $stripped_input =~ s/ ^ =cut \n (.) /$1/mgx;

    $input = $stripped_input if $stripped_input ne "";
    if ($input !~ / ^ =pod /x) {
      $input =~ s/ ^ \s+ //x;
      $input = "=pod\n\n$input";
    }
    if ($input !~ / =cut $ /x) {
      $input =~ s/ \s+ $ //x;
      $input .= "\n\n=cut\n";
    }

    my $msg = "got expected output for $file";
    if ($output eq $input) {
        pass($msg);
    }
    elsif ($ENV{PERL_TEST_DIFF}) {
      fail($msg);
      require File::Temp;
      my $orig_file = File::Temp->new();
      local $/ = "\n";
      chomp $input;
      print $orig_file $input, "\n";
      close $orig_file || die "Can't close orig_file: $!";

      chomp $output;
      my $parsed_file = File::Temp->new();
      print $parsed_file $output, "\n";
      close $parsed_file || die "Can't close parsed_file";

      my $diff = File::Temp->new();
      system("$ENV{PERL_TEST_DIFF} $orig_file $parsed_file > $diff");

      open my $fh, "<", $diff || die "Can't open $diff";
      my @diffs = <$fh>;
      diag(@diffs);
    }
    else {
        eval { require Text::Diff; };
        if ($@) {
            is($output, $input, $msg);
            diag("Set environment variable PERL_TEST_DIFF=diff_tool or install"
               . " Text::Diff to see just the differences.");
        }
        else {
            fail($msg);
            diag Text::Diff::diff(\$input, \$output, { STYLE => 'Unified' });
        }
    }
  }
}
