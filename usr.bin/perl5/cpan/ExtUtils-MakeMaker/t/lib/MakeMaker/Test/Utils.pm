package MakeMaker::Test::Utils;

use File::Spec;
use strict;
use warnings;
use Config;
use Cwd qw(getcwd);
use Carp qw(croak);
use File::Path;
use File::Basename;

require Exporter;
our @ISA = qw(Exporter);

our $Is_VMS     = $^O eq 'VMS';
our $Is_MacOS   = $^O eq 'MacOS';
our $Is_FreeBSD = $^O eq 'freebsd';

our @EXPORT = qw(which_perl perl_lib makefile_name makefile_backup
                 make make_run run make_macro calibrate_mtime
                 have_compiler slurp
                 $Is_VMS $Is_MacOS
                 run_ok
                 hash2files
                 in_dir
                );


# Setup the code to clean out %ENV
{
    # Environment variables which might effect our testing
    my @delete_env_keys = qw(
        PERL_MM_OPT
        PERL_MM_USE_DEFAULT
        HARNESS_TIMER
        HARNESS_OPTIONS
        HARNESS_VERBOSE
        PREFIX
        MAKEFLAGS
        PERL_INSTALL_QUIET
    );

    my %default_env_keys;

    # Inform the BSDPAN hacks not to register modules installed for testing.
    $default_env_keys{PORTOBJFORMAT} = 1 if $Is_FreeBSD;

    # https://github.com/Perl-Toolchain-Gang/ExtUtils-MakeMaker/issues/65
    $default_env_keys{ACTIVEPERL_CONFIG_SILENT} = 1;

    # Remember the ENV values because on VMS %ENV is global
    # to the user, not the process.
    my %restore_env_keys;

    sub clean_env {
        for my $key (keys %default_env_keys) {
            $ENV{$key} = $default_env_keys{$key} unless $ENV{$key};
        }

        for my $key (@delete_env_keys) {
            if( exists $ENV{$key} ) {
                $restore_env_keys{$key} = delete $ENV{$key};
            }
            else {
                delete $ENV{$key};
            }
        }
    }

    END {
        while( my($key, $val) = each %restore_env_keys ) {
            $ENV{$key} = $val;
        }
    }
}
clean_env();


=head1 NAME

MakeMaker::Test::Utils - Utility routines for testing MakeMaker

=head1 SYNOPSIS

  use MakeMaker::Test::Utils;

  my $perl     = which_perl;
  perl_lib;

  my $makefile      = makefile_name;
  my $makefile_back = makefile_backup;

  my $make          = make;
  my $make_run      = make_run;
  make_macro($make, $targ, %macros);

  my $mtime         = calibrate_mtime;

  my $out           = run($cmd);

  my $have_compiler = have_compiler();

  my $text          = slurp($filename);


=head1 DESCRIPTION

A consolidation of little utility functions used throughout the
MakeMaker test suite.

=head2 Functions

The following are exported by default.

=over 4

=item B<which_perl>

  my $perl = which_perl;

Returns a path to perl which is safe to use in a command line, no
matter where you chdir to.

=cut

sub which_perl {
    my $perl = $^X;
    $perl ||= 'perl';

    # VMS should have 'perl' aliased properly
    return $perl if $Is_VMS;

    $perl .= $Config{exe_ext} unless $perl =~ m/$Config{exe_ext}$/i;

    my $perlpath = File::Spec->rel2abs( $perl );
    unless( $Is_MacOS || -x $perlpath ) {
        # $^X was probably 'perl'

        # When building in the core, *don't* go off and find
        # another perl
        die "Can't find a perl to use (\$^X=$^X), (\$perlpath=$perlpath)"
          if $ENV{PERL_CORE};

        foreach my $path (File::Spec->path) {
            $perlpath = File::Spec->catfile($path, $perl);
            last if -x $perlpath;
        }
    }
    $perlpath = qq{"$perlpath"}; # "safe... in a command line" even with spaces

    return $perlpath;
}

=item B<perl_lib>

  perl_lib;

Sets up environment variables so perl can find its libraries.

=cut

my $old5lib = $ENV{PERL5LIB};
my $had5lib = exists $ENV{PERL5LIB};
sub perl_lib {
    my $basecwd = (File::Spec->splitdir(getcwd))[-1];
    croak "Basename of cwd needs to be 't' but is '$basecwd'\n"
        unless $basecwd eq 't';
                               # perl-src/t/
    my $lib =  $ENV{PERL_CORE} ? qq{../lib}
                               # ExtUtils-MakeMaker/t/
                               : qq{../blib/lib};
    $lib = File::Spec->rel2abs($lib);
    my @libs = ($lib);
    push @libs, $ENV{PERL5LIB} if exists $ENV{PERL5LIB};
    $ENV{PERL5LIB} = join($Config{path_sep}, @libs);
    unshift @INC, $lib;
}

END {
    if( $had5lib ) {
        $ENV{PERL5LIB} = $old5lib;
    }
    else {
        delete $ENV{PERL5LIB};
    }
}


=item B<makefile_name>

  my $makefile = makefile_name;

MakeMaker doesn't always generate 'Makefile'.  It returns what it
should generate.

=cut

sub makefile_name {
    return $Is_VMS ? 'Descrip.MMS' : 'Makefile';
}

=item B<makefile_backup>

  my $makefile_old = makefile_backup;

Returns the name MakeMaker will use for a backup of the current
Makefile.

=cut

sub makefile_backup {
    my $makefile = makefile_name;
    return $Is_VMS ? "$makefile".'_old' : "$makefile.old";
}

=item B<make>

  my $make = make;

Returns a good guess at the make to run.

=cut

sub make {
    my $make = $Config{make};
    $make = $ENV{MAKE} if exists $ENV{MAKE};

    return $Is_VMS ? $make : qq{"$make"};
}

=item B<make_run>

  my $make_run = make_run;

Returns the make to run as with make() plus any necessary switches.

=cut

sub make_run {
    my $make = make;
    $make .= ' -nologo' if $make eq 'nmake';

    return $make;
}

=item B<make_macro>

    my $make_cmd = make_macro($make, $target, %macros);

Returns the command necessary to run $make on the given $target using
the given %macros.

  my $make_test_verbose = make_macro(make_run(), 'test',
                                     TEST_VERBOSE => 1);

This is important because VMS's make utilities have a completely
different calling convention than Unix or Windows.

%macros is actually a list of tuples, so the order will be preserved.

=cut

sub make_macro {
    my($make, $target) = (shift, shift);

    my $is_mms = $make =~ /^MM(K|S)/i;

    my @macros;
    while( my($key,$val) = splice(@_, 0, 2) ) {
        push @macros, qq{$key=$val};
    }
    my $macros = '';
    if (scalar(@macros)) {
        if ($is_mms) {
            map { $_ = qq{"$_"} } @macros;
            $macros = '/MACRO=(' . join(',', @macros) . ')';
        }
        else {
            $macros = join(' ', @macros);
        }
    }

    return $is_mms ? "$make$macros $target" : "$make $target $macros";
}

=item B<calibrate_mtime>

  my $mtime = calibrate_mtime;

When building on NFS, file modification times can often lose touch
with reality.  This returns the mtime of a file which has just been
touched.

=cut

sub calibrate_mtime {
    my $file = "calibrate_mtime-$$.tmp";
    open(FILE, ">$file") || die $!;
    print FILE "foo";
    close FILE;
    my($mtime) = (stat($file))[9];
    unlink $file;
    return $mtime;
}

=item B<run>

  my $out = run($command);
  my @out = run($command);

Runs the given $command as an external program returning at least STDOUT
as $out.  If possible it will return STDOUT and STDERR combined as you
would expect to see on a screen.

=cut

sub run {
    my $cmd = shift;

    use ExtUtils::MM;

    # Unix, modern Windows and OS/2 from 5.005_54 up can handle 2>&1
    # This makes our failure diagnostics nicer to read.
    if (MM->can_redirect_error) {
        return `$cmd 2>&1`;
    }
    else {
        return `$cmd`;
    }
}


=item B<run_ok>

  my @out = run_ok($cmd);

Like run() but it tests that the result exited normally.

The output from run() will be used as a diagnostic if it fails.

=cut

sub run_ok {
    my $tb = Test::Builder->new;

    my @out = run(@_);

    $tb->cmp_ok( $?, '==', 0, "run(@_)" ) || $tb->diag(@out);

    return wantarray ? @out : join "", @out;
}

=item have_compiler

  $have_compiler = have_compiler;

Returns true if there is a compiler available for XS builds.

=cut

sub have_compiler {
    return 1 if $ENV{PERL_CORE};

    my $have_compiler = 0;

    in_dir(sub {
        eval {
            require ExtUtils::CBuilder;
            my $cb = ExtUtils::CBuilder->new(quiet=>1);
            $have_compiler = $cb->have_compiler;
        };
    });

    return $have_compiler;
}

=item slurp

  $contents = slurp($filename);

Returns the $contents of $filename.

Will die if $filename cannot be opened.

=cut

sub slurp {
    my $filename = shift;

    local $/ = undef;
    open my $fh, $filename or die "Can't open $filename for reading: $!";
    my $text = <$fh>;
    close $fh;

    return $text;
}

=item hash2files

  hash2files('dirname', { 'filename' => 'some content' });

Goes through given hash-ref, treating each key as a /-separated filename
under the specified directory, and writing the value into it. Will create
any necessary directories.

Will die if errors occur.

=cut

sub hash2files {
    my ($prefix, $hashref) = @_;
    while(my ($file, $text) = each %$hashref) {
        # Convert to a relative, native file path.
        $file = File::Spec->catfile(File::Spec->curdir, $prefix, split m{\/}, $file);
        my $dir = dirname($file);
        mkpath $dir;
        my $utf8 = ("$]" < 5.008 or !$Config{useperlio}) ? "" : ":utf8";
        open(FILE, ">$utf8", $file) || die "Can't create $file: $!";
        print FILE $text;
        close FILE;
        # ensure file at least 1 second old for makes that assume
        # files with the same time are out of date.
        my $time = calibrate_mtime();
        utime $time, $time - 1, $file;
    }
}

=item in_dir

  $retval = in_dir(\&coderef);
  $retval = in_dir(\&coderef, $specified_dir);
  $retval = in_dir { somecode(); };
  $retval = in_dir { somecode(); } $specified_dir;

Does a C<chdir> to either a directory. If none is specified, one is
created with L<File::Temp> and then automatically deleted after. It ends
by C<chdir>ing back to where it started.

If the given code throws an exception, it will be re-thrown after the
re-C<chdir>.

Returns the return value of the given code.

=cut

sub in_dir(&;$) {
    my $code = shift;
    require File::Temp;
    my $dir = shift || File::Temp::tempdir(TMPDIR => 1, CLEANUP => 1);
    # chdir to the new directory
    my $orig_dir = getcwd();
    chdir $dir or die "Can't chdir to $dir: $!";
    # Run the code, but trap the error so we can chdir back
    my $return;
    my $ok = eval { $return = $code->(); 1; };
    my $err = $@;
    # chdir back
    chdir $orig_dir or die "Can't chdir to $orig_dir: $!";
    # rethrow if necessary
    die $err unless $ok;
    return $return;
}

=back

=head1 AUTHOR

Michael G Schwern <schwern@pobox.com>

=cut

1;
