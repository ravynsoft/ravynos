package PrimitiveCapture;
use strict;
use warnings;

if ("$]" >= 5.008000) {
  eval "#line @{[__LINE__+1]} ".q{"lib/PrimitiveCapture.pm"
    sub capture_stdout {
      my $sub = shift;
      my $stdout;
      open my $oldout, ">&STDOUT" or die "Can't dup STDOUT: $!";
      close STDOUT;
      open STDOUT, '>', \$stdout or die "Can't open STDOUT: $!";
      $sub->();
      close STDOUT;
      open STDOUT, ">&", $oldout or die "Can't dup \$oldout: $!";
      return $stdout;
    }
    sub capture_stderr {
      my $sub = shift;
      my $stderr;
      open my $olderr, ">&STDERR" or die "Can't dup STDERR: $!";
      close STDERR;
      open STDERR, '>', \$stderr or die "Can't open STDERR: $!";
      $sub->();
      close STDERR;
      open STDERR, ">&", $olderr or die "Can't dup \$olderr: $!";
      return $stderr;
    }
  }; die $@ unless $@ eq "";
} else {
  eval "#line @{[__LINE__+1]} ".q{"lib/PrimitiveCapture.pm"
    use File::Spec;
    use File::Temp;
    my $tmpdir;
    my $i = 0;
    sub _tmpfile {
      $tmpdir ||= File::Temp::tempdir(CLEANUP => 1, TMPDIR => 1);
      return File::Spec->catfile($tmpdir, $i++);
    }
    sub _slurp {
      my $filename = shift;
      open my $fh, "<", $filename or die "Can't read $filename: $!";
      local $/ = undef;
      my $content = <$fh>;
      defined $content or die "Can't read $filename: $!";
      return $content;
    }
    sub capture_stdout {
      my $sub = shift;
      my $tmpfile = _tmpfile();
      local *OLDSTDOUT;
      open OLDSTDOUT, ">&STDOUT" or die "Can't dup STDOUT: $!";
      close STDOUT;
      open STDOUT, '>', $tmpfile or die "Can't open STDOUT: $!";
      $sub->();
      close STDOUT;
      open STDOUT, ">&OLDSTDOUT" or die "Can't dup OLDSTDOUT: $!";
      close OLDSTDOUT;
      return _slurp($tmpfile);
    }
    sub capture_stderr {
      my $sub = shift;
      my $tmpfile = _tmpfile();
      local *OLDSTDERR;
      open OLDSTDERR, ">&STDERR" or die "Can't dup STDERR: $!";
      close STDERR;
      open STDERR, '>', $tmpfile or die "Can't open STDERR: $!";
      $sub->();
      close STDERR;
      open STDERR, ">&OLDSTDERR" or die "Can't dup OLDSTDERR: $!";
      close OLDSTDERR;
      return _slurp($tmpfile);
    }
  }; die $@ unless $@ eq "";
}

1;
