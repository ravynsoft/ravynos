package FilePathTest;
use strict;
use warnings;
use base 'Exporter';
use SelectSaver;
use Carp;
use Cwd;
use File::Spec::Functions;
use File::Path ();
use Test::More ();

our @EXPORT_OK = qw(
    _run_for_warning
    _run_for_verbose
    _cannot_delete_safe_mode
    _verbose_expected
    create_3_level_subdirs
    cleanup_3_level_subdirs
);

sub _basedir {
  return catdir(
      curdir(),
      sprintf( 'test-%x-%x-%x', time, $$, rand(99999) ),
  );
}

sub _run_for_warning {
  my $coderef = shift;
  my $warn = '';
  local $SIG{__WARN__} = sub { $warn .= shift };
  &$coderef;
  return $warn;
}

sub _run_for_verbose {
  my $coderef = shift;
  my $stdout = '';
  {
    my $guard = SelectSaver->new(_ref_to_fh(\$stdout));
    &$coderef;
  }
  return $stdout;
}

sub _ref_to_fh {
  my $output = shift;
  open my $fh, '>', $output;
  return $fh;
}

# Whether a directory can be deleted without modifying permissions varies
# by platform and by current privileges, so we really have to do the same
# check the module does in safe mode to determine that.

sub _cannot_delete_safe_mode {
  my $path = shift;
  return $^O eq 'VMS'
         ? !&VMS::Filespec::candelete($path)
         : !-w $path;
}

# What verbose mode reports depends on what it can do in safe mode.
# Plus on VMS, mkpath may report what it's operating on in a
# different format from the format of the path passed to it.

sub _verbose_expected {
  my ($function, $path, $safe_mode, $base) = @_;
  my $expected;

  if ($function =~ m/^(mkpath|make_path)$/) {
    # On VMS, mkpath reports in Unix format.  Maddeningly, it
    # reports the top-level directory without a trailing slash
    # and everything else with.
    if ($^O eq 'VMS') {
      $path = VMS::Filespec::unixify($path);
      $path =~ s/\/$// if defined $base && $base;
    }
    $expected = "mkdir $path\n";
  }
  elsif ($function =~ m/^(rmtree|remove_tree)$/) {
    # N.B. Directories must still/already exist for this to work.
    $expected = $safe_mode && _cannot_delete_safe_mode($path)
              ? "skipped $path\n"
              : "rmdir $path\n";
  }
  elsif ($function =~ m/^(unlink)$/) {
    $expected = "unlink $path\n";
    $expected =~ s/\n\z/\.\n/ if $^O eq 'VMS';
  }
  else {
    die "Unknown function $function in _verbose_expected";
  }
  return $expected;
}

BEGIN {
  if ($] < 5.008000) {
    eval qq{#line @{[__LINE__+1]} "@{[__FILE__]}"\n} . <<'END' or die $@;
      no warnings 'redefine';
      use Symbol ();

      sub _ref_to_fh {
        my $output = shift;
        my $fh = Symbol::gensym();
        tie *$fh, 'StringIO', $output;
        return $fh;
      }

      package StringIO;
      sub TIEHANDLE { bless [ $_[1] ], $_[0] }
      sub CLOSE    { @{$_[0]} = (); 1 }
      sub PRINT    { ${ $_[0][0] } .= $_[1] }
      sub PRINTF   { ${ $_[0][0] } .= sprintf $_[1], @_[2..$#_] }
      1;
END
  }
}

sub create_3_level_subdirs {
    my @dirnames = @_;
    my %seen = map {$_ => 1} @dirnames;
    croak "Need 3 distinct names for subdirectories"
        unless scalar(keys %seen) == 3;
    my $tdir = File::Spec::Functions::tmpdir();
    my $least_deep      = catdir($tdir, $dirnames[0]);
    my $next_deepest    = catdir($least_deep, $dirnames[1]);
    my $deepest         = catdir($next_deepest, $dirnames[2]);
    return ($least_deep, $next_deepest, $deepest);
}

sub cleanup_3_level_subdirs {
    # runs 2 tests
    my $least_deep = shift;
    croak "Must provide path of least subdirectory"
        unless (length($least_deep) and (-d $least_deep));
    my $x;
    my $opts = { error => \$x };
    File::Path::remove_tree($least_deep, $opts);
    Test::More::ok(! -d $least_deep, "directory '$least_deep' removed, as expected");
    Test::More::is(scalar(@{$x}), 0, "no error messages using remove_tree() with \$opts");
}

1;
