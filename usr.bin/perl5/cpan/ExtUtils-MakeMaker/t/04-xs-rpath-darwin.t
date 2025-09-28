#!/usr/bin/perl -w

# This test file tests a special case for the generation of XS modules on OS darwin.
#  More specifically, it tests if we are able to compile an XS module which refers
#  to another shared library in a non-standard location such that we can
#  load the XS module from a perl script without having to set the
#  DYLD_LIBRARY_PATH environment variable. See PR #403 and issue #402.
#
package Main;
use strict;
use warnings;
use Config;
BEGIN {
    chdir 't' or die "chdir(t): $!\n";
    unshift @INC, 'lib/';
    use Test::More;
    if( $^O ne "darwin" ) {
        plan skip_all => 'Not darwin platform';
    }
    else {
        plan skip_all => 'Dynaloading not enabled'
            if !$Config{usedl} or $Config{usedl} ne 'define';
        plan tests => 1;
    }
}
use Cwd;
use ExtUtils::MakeMaker;
use File::Temp qw[tempdir];
use File::Path;  # exports: mkpath and rmtree
use File::Spec;

{
    $| = 1;
    # We need this when re-running "perl Makefile.PL"
    my $ext_utils_lib_dir = File::Spec->rel2abs('../lib');
    # This tmpdir will be removed when the program exits
    my $tmpdir = tempdir( DIR => '.', CLEANUP => 1 );
    my $cwd = getcwd;
    # File::Temp will not clean up the temp directory if the current directory
    #   is a sub directory of the temp dir. This can happen in the case of an
    #   error (a call to die). which disrupts the normal program flow that would
    #   have restored the cwd before exit. To solve this issue
    #   we add the below END block (which will be called before the File::Temp
    #   cleanup END block call since END blocks are called in LIFO order)
    END { chdir $cwd }
    _chdir($tmpdir);
    my $self = Main->new(
        mylib_dir => "mylib",
        mylib_c_fn => "mylib.c",
        mylib_h_fn => "mylib.h",
        mylib_lib_name => "mylib",
        module_name => "My::Module",
        test_script_name => 'p.pl',
        ext_utils_lib_dir => $ext_utils_lib_dir,
    );
    $self->compile_library();
    $self->write_makefile_pl();
    $self->write_module_file();
    $self->write_xs_file();
    $self->run_make();
    $self->write_test_script();
    $self->run_test_script();
    _chdir($cwd);
}

sub _chdir { chdir $_[0] or die "Cannot change directory to $_[0] : $!" }

sub _mkpath { mkpath($_[0]) or die "Could not create directory $_[0] : $!" };

sub run_test_script {
    my ($self) = @_;

    my @cmd = ($^X, '-Mblib', $self->{test_script_name});
    my $out = _capture_stdout(\@cmd);
    like( $out, qr{\Qcalling foo()\E\s+\QHello from foo()\E});
}

sub write_xs_file {
    my ($self) = @_;

    my $str = <<'END';
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "mylib.h"

MODULE = <<module_name_colon>>  PACKAGE = <<module_name_colon>>
PROTOTYPES: DISABLE

void
mylib_func()
  CODE:
    printf("calling foo()\n");
    foo();
END
    $str =~ s/\Q<<module_name_colon>>\E/$self->{module_name}/g;
    my @module_name = split /::/, $self->{module_name};
    my $xs_name = pop @module_name;
    $xs_name .= '.xs';
    _write_file( $xs_name, $str );
}

sub write_test_script {
    my ($self) = @_;

    my $str = <<'END';
use strict;
use warnings;
use ExtUtils::testlib;
use <<module_name_colon>>;

<<module_name_colon>>::mylib_func();
END
    $str =~ s/\Q<<module_name_colon>>\E/$self->{module_name}/g;
    _write_file( $self->{test_script_name}, $str );
}

sub run_make {
    my ($self) = @_;

    my @cmd = ($^X, '-I'. $self->{ext_utils_lib_dir}, 'Makefile.PL');
    _run_system_cmd(\@cmd);
    _run_system_cmd(['make']);
}

sub write_module_file {
    my ( $self ) = @_;

    my @dirs = split /::/, $self->{module_name};
    my $basename = pop @dirs;
    my $dir = File::Spec->catfile('lib', @dirs);
    _mkpath( $dir );
    my $fn = File::Spec->catfile($dir, $basename . '.pm');
    my $str = <<'END';
package <<module_name_colon>>;
require Exporter;
require DynaLoader;
$VERSION = 1.01;
@ISA    = qw(Exporter DynaLoader);
@EXPORT = qw();
bootstrap <<module_name_colon>> $VERSION;
1;

=head1 NAME

<<module_name_colon>> - Short description of <<module_name_colon>>
END
    $str =~ s/\Q<<module_name_colon>>\E/$self->{module_name}/g;
    _write_file( $fn, $str );
}

sub write_makefile_pl {
    my ( $self ) = @_;

    my $str = <<'END';
use strict;
use warnings;
use ExtUtils::MakeMaker;

WriteMakefile(
  NAME          => '<<module_name_colon>>',
  VERSION_FROM  => 'lib/<<module_name_slash>>.pm',
  ABSTRACT_FROM => 'lib/<<module_name_slash>>.pm',
  PERL          => "$^X -w",
  LIBS          => ['-L./<<lib_dir>> -l<<lib_name>>'],
  INC           => '-I. -I./<<lib_dir>>',
);
END
    my $mod_name1 = $self->{module_name};
    my $mod_name2 = $self->{module_name};
    $mod_name2 =~ s{::}{/}g;
    $str =~ s/\Q<<module_name_colon>>\E/$mod_name1/g;
    $str =~ s/\Q<<module_name_slash>>\E/$mod_name2/g;
    $str =~ s/\Q<<lib_dir>>\E/$self->{mylib_dir}/g;
    $str =~ s/\Q<<lib_name>>\E/$self->{mylib_lib_name}/g;
    _write_file('Makefile.PL', $str);
}

sub compile_library {
    my ($self) = @_;

    _mkpath( $self->{mylib_dir} );
    my $cwd = getcwd;
    _chdir( $self->{mylib_dir} );
    $self->write_mylib_h();
    $self->write_mylib_c();
    $self->compile_mylib();
    _chdir( $cwd );
}

sub compile_mylib {
    my ($self) = @_;

    my $cc = $Config{cc};
    my $libext = $Config{so};

    my $libname = 'lib' . $self->{mylib_lib_name} . '.' . $libext;
    my @cmd = ($cc, '-I.', '-dynamiclib', '-install_name',
             '@rpath/' . $libname,
             'mylib.c', '-o', $libname);
    _run_system_cmd(\@cmd);
}

sub _capture_stdout {
    my ($cmd) = @_;

    my $out = `@$cmd`;
    _check_sys_cmd_error( $cmd, $? ) if $? != 0;
    return $out;
}

sub _stringify_cmd { '"' . (join " ", @{$_[0]}) . '"' }

sub _check_sys_cmd_error {
    my ( $cmd, $error ) = @_;
    my $cmd_str = _stringify_cmd($cmd);
    if ( $error == -1 ) {
        # A return value of -1 from system() indicates a failure to start the program
        die "Could not run $cmd_str: $!";
    }
    elsif ($error & 127) {
        die sprintf "Command $cmd_str : killed by signal %d, %s coredump\n",
          ($error & 127),  ($error & 128) ? 'with' : 'without';
    }
    elsif ($error != 0) {
        die sprintf "$cmd_str exited with error code %d\n", $error >> 8;
    }
}

sub _run_system_cmd {
    my ($cmd) = @_;

    my $res = system @$cmd;
    _check_sys_cmd_error( $cmd, $res ) if $res != 0;

}

sub write_mylib_c {
    my ($self) = @_;
    my $str = <<'END';
#include <stdio.h>
#include <stdlib.h>
#include "mylib.h"

void foo() {
    printf( "Hello from foo()\n");
}
END
    _write_file($self->{mylib_c_fn}, $str);
}

sub write_mylib_h {
    my ($self) = @_;
    my $str = 'void foo();';
    _write_file($self->{mylib_h_fn}, $str);
}

sub _write_file {
    my ($file, $text) = @_;
    my $utf8 = ("$]" < 5.008 or !$Config{useperlio}) ? "" : ":utf8";
    open(FILE, ">$utf8", $file) || die "Can't create $file: $!";
    print FILE $text;
    close FILE;
}

sub new {
    my ($class, %args) = @_;
    return bless \%args, $class;
}
