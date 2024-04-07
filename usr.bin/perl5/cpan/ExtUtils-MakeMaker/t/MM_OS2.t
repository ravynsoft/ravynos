#!/usr/bin/perl -w

BEGIN {
    unshift @INC, 't/lib';
}
chdir 't';

use strict;
use warnings;
use Test::More;
if ($^O =~ /os2/i) {
	plan( tests => 32 );
} else {
	plan( skip_all => "This is not OS/2" );
}

# for dlsyms, overridden in tests
BEGIN {
	package ExtUtils::MM_OS2;
	use subs 'system', 'unlink';
}

# for maybe_command
use File::Spec;

use_ok( 'ExtUtils::MM_OS2' );
ok( grep( 'ExtUtils::MM_OS2',  @MM::ISA),
	'ExtUtils::MM_OS2 should be parent of MM' );

# dlsyms
my $mm = bless({
	SKIPHASH => {
		dynamic => 1
	},
	NAME => 'foo:bar::',
}, 'ExtUtils::MM_OS2');

is( $mm->dlsyms(), '',
	'dlsyms() should return nothing with dynamic flag set' );

$mm->{BASEEXT} = 'baseext';
delete $mm->{SKIPHASH};
my $res = $mm->dlsyms();
like( $res, qr/baseext\.def: Makefile/,
	'... without flag, should return make targets' );
like( $res, qr/"DL_FUNCS" => \{  \}/,
	'... should provide empty hash refs where necessary' );
like( $res, qr/"DL_VARS" => \[]/, '... and empty array refs too' );

$mm->{FUNCLIST} = 'funclist';
$res = $mm->dlsyms( IMPORTS => 'imports' );
like( $res, qr/"FUNCLIST" => .+funclist/,
	'... should pick up values from object' );
like( $res, qr/"IMPORTS" => .+imports/, '... and allow parameter options too' );

my $can_write;
{
	local *OUT;
	$can_write = open(OUT, '>tmp_imp');
}

SKIP: {
	skip("Cannot write test files: $!", 7) unless $can_write;

	$mm->{IMPORTS} = { foo => 'bar' };

	local $@;
	eval { $mm->dlsyms() };
	like( $@, qr/Can.t mkdir tmp_imp/,
		'... should die if directory cannot be made' );

	unlink('tmp_imp') or skip("Cannot remove test file: $!", 9);
	eval { $mm->dlsyms() };
	like( $@, qr/Malformed IMPORT/, 'should die from malformed import symbols');

	$mm->{IMPORTS} = { foo => 'bar.baz' };

	my @sysfail = ( 1, 0, 1 );
	my ($sysargs, $unlinked);

	*ExtUtils::MM_OS2::system = sub {
		$sysargs = shift;
		return shift @sysfail;
	};

	*ExtUtils::MM_OS2::unlink = sub {
		$unlinked++;
	};

	eval { $mm->dlsyms() };

	like( $sysargs, qr/^emximp/, '... should try to call system() though' );
	like( $@, qr/Cannot make import library/,
		'... should die if emximp syscall fails' );

	# sysfail is 0 now, call emximp call should succeed
	eval { $mm->dlsyms() };
	is( $unlinked, 1, '... should attempt to unlink temp files' );
	like( $@, qr/Cannot extract import/,
		'... should die if other syscall fails' );

	# make both syscalls succeed
	@sysfail = (0, 0);
	local $@;
	eval { $mm->dlsyms() };
	is( $@, '', '... should not die if both syscalls succeed' );
}

# static_lib
{
	my $called = 0;

	# avoid "used only once"
	local *ExtUtils::MM_Unix::static_lib;
	*ExtUtils::MM_Unix::static_lib = sub {
		$called++;
		return "\n\ncalled static_lib\n\nline2\nline3\n\nline4";
	};

	my $args = bless({ IMPORTS => {}, }, 'MM');

	# without IMPORTS as a populated hash, there will be no extra data
	my $ret = ExtUtils::MM_OS2::static_lib( $args );
	is( $called, 1, 'static_lib() should call parent method' );
	like( $ret, qr/^called static_lib/m,
		'... should return parent data unless IMPORTS exists' );

	$args->{IMPORTS} = { foo => 1};
	$ret = ExtUtils::MM_OS2::static_lib( $args );
	is( $called, 2, '... should call parent method if extra imports passed' );
	like( $ret, qr/^called static_lib\n\t\$\(AR\) \$\(AR_STATIC_ARGS\)/m,
		'... should append make tags to first line from parent method' );
	like( $ret, qr/\$@\n\n\nline2\nline3\n\nline4/m,
		'... should include remaining data from parent method' );

}

# replace_manpage_separator
my $sep = '//a///b//c/de';
is( ExtUtils::MM_OS2->replace_manpage_separator($sep), '.a.b.c.de',
	'replace_manpage_separator() should turn multiple slashes into periods' );

# maybe_command
{
	local *DIR;
	my ($dir, $noext, $exe, $cmd);
	my $found = 0;

	my ($curdir, $updir) = (File::Spec->curdir, File::Spec->updir);

	# we need:
	#	1) a directory
	#	2) an executable file with no extension
	# 	3) an executable file with the .exe extension
	# 	4) an executable file with the .cmd extension
	# we assume there will be one somewhere in the path
	# in addition, we need them to be unique enough they do not trip
	# an earlier file test in maybe_command().  Portability.

	foreach my $path (split(/:/, $ENV{PATH})) {
		opendir(DIR, $path) or next;
		while (defined(my $file = readdir(DIR))) {
			next if $file eq $curdir or $file eq $updir;
			$file = File::Spec->catfile($path, $file);
			unless (defined $dir) {
				if (-d $file) {
					next if ( -x $file . '.exe' or -x $file . '.cmd' );

					$dir = $file;
					$found++;
				}
			}
			if (-x $file) {
				my $ext;
				if ($file =~ s/\.(exe|cmd)\z//) {
					$ext = $1;

					# skip executable files with names too similar
					next if -x $file;
					$file .= '.' . $ext;

				} else {
					unless (defined $noext) {
						$noext = $file;
						$found++;
					}
					next;
				}

				unless (defined $exe) {
					if ($ext eq 'exe') {
						$exe = $file;
						$found++;
						next;
					}
				}
				unless (defined $cmd) {
					if ($ext eq 'cmd') {
						$cmd = $file;
						$found++;
						next;
					}
				}
			}
			last if $found == 4;
		}
		last if $found == 4;
	}

	SKIP: {
		skip('No appropriate directory found', 1) unless defined $dir;
		is( ExtUtils::MM_OS2->maybe_command( $dir ), undef,
			'maybe_command() should ignore directories' );
	}

	SKIP: {
		skip('No non-exension command found', 1) unless defined $noext;
		is( ExtUtils::MM_OS2->maybe_command( $noext ), $noext,
			'maybe_command() should find executable lacking file extension' );
	}

	SKIP: {
		skip('No .exe command found', 1) unless defined $exe;
		(my $noexe = $exe) =~ s/\.exe\z//;
		is( ExtUtils::MM_OS2->maybe_command( $noexe ), $exe,
			'maybe_command() should find .exe file lacking extension' );
	}

	SKIP: {
		skip('No .cmd command found', 1) unless defined $cmd;
		(my $nocmd = $cmd) =~ s/\.cmd\z//;
		is( ExtUtils::MM_OS2->maybe_command( $nocmd ), $cmd,
			'maybe_command() should find .cmd file lacking extension' );
	}
}

# file_name_is_absolute
ok( ExtUtils::MM_OS2->file_name_is_absolute( 's:/' ),
	'file_name_is_absolute() should be true for paths with volume and slash' );
ok( ExtUtils::MM_OS2->file_name_is_absolute( '\foo' ),
	'... and for paths with leading slash but no volume' );
ok( ! ExtUtils::MM_OS2->file_name_is_absolute( 'arduk' ),
	'... but not for paths with no leading slash or volume' );


$mm->init_linker;

# PERL_ARCHIVE
is( $mm->{PERL_ARCHIVE}, '$(PERL_INC)/libperl$(LIB_EXT)', 'PERL_ARCHIVE' );

# PERL_ARCHIVE_AFTER
{
	my $aout = 0;
	local *OS2::is_aout;
	*OS2::is_aout = \$aout;

        $mm->init_linker;
	isnt( $mm->{PERL_ARCHIVE_AFTER}, '',
		'PERL_ARCHIVE_AFTER should be empty without $is_aout set' );
	$aout = 1;
	is( $mm->{PERL_ARCHIVE_AFTER},
            '$(PERL_INC)/libperl_override$(LIB_EXT)',
		'... and has libperl_override if it is set' );
}

# EXPORT_LIST
is( $mm->{EXPORT_LIST}, '$(BASEEXT).def',
	'EXPORT_LIST should add .def to BASEEXT member' );

END {
	use File::Path;
	rmtree('tmp_imp') if -e 'tmp_imp';
	unlink 'tmpimp.imp';
}
