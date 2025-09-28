#!/usr/bin/perl -wT

use strict;
use warnings;

use Config;
push @INC, '.';
if (-f 't/test.pl') {
  require './t/test.pl';
} else {
  require '../../t/test.pl';
}

my %modules;

my $db_file;
BEGIN {
    use Config;
    foreach (qw/SDBM_File GDBM_File ODBM_File NDBM_File DB_File/) {
        if ($Config{extensions} =~ /\b$_\b/) {
            $db_file = $_;
            last;
        }
    }
}

%modules = (
   # ModuleName   => q| code to check that it was loaded |,
    'List::Util'  => q| ::is( ref List::Util->can('first'), 'CODE' ) |,  # 5.7.2
    'Cwd'         => q| ::is( ref Cwd->can('fastcwd'),'CODE' ) |,         # 5.7 ?
    'File::Glob'  => q| ::is( ref File::Glob->can('doglob'),'CODE' ) |,   # 5.6
    $db_file      => q| ::is( ref $db_file->can('TIEHASH'), 'CODE' ) |,  # 5.0
    'Socket'      => q| ::is( ref Socket->can('inet_aton'),'CODE' ) |,    # 5.0
    'Time::HiRes' => q| ::is( ref Time::HiRes->can('usleep'),'CODE' ) |,  # 5.7.3
);

plan (26 + keys(%modules) * 3);

# Try to load the module
use_ok( 'DynaLoader' );

# Some tests need to be skipped on old Darwin versions.
# Commit ce12ed1954 added the skip originally, without specifying which
# darwin version needed it.  I know OS X 10.6 (Snow Leopard; darwin 10)
# supports it, so skip anything before that.
my $old_darwin = $^O eq 'darwin' && ($Config{osvers} =~ /^(\d+)/)[0] < 10;

# Check functions
can_ok( 'DynaLoader' => 'bootstrap'               ); # defined in Perl section
can_ok( 'DynaLoader' => 'dl_load_flags'           ); # defined in Perl section
can_ok( 'DynaLoader' => 'dl_error'                ); # defined in XS section
if ($Config{usedl}) {
    can_ok( 'DynaLoader' => 'dl_find_symbol'      ); # defined in XS section
    can_ok( 'DynaLoader' => 'dl_install_xsub'     ); # defined in XS section
    can_ok( 'DynaLoader' => 'dl_load_file'        ); # defined in XS section
    can_ok( 'DynaLoader' => 'dl_undef_symbols'    ); # defined in XS section
    SKIP: {
        skip( "unloading unsupported on $^O", 1 ) if ($old_darwin || $^O eq 'VMS');
        can_ok( 'DynaLoader' => 'dl_unload_file'  ); # defined in XS section
    }
} else {
    foreach my $symbol (qw(dl_find_symbol dl_install_sub dl_load_file
			   dl_undef_symbols dl_unload_file)) {
	is(DynaLoader->can($symbol), undef,
	   "Without dynamic loading, DynaLoader should not have $symbol");
    }
}

can_ok( 'DynaLoader' => 'dl_expandspec'           );
can_ok( 'DynaLoader' => 'dl_findfile'             );
can_ok( 'DynaLoader' => 'dl_find_symbol_anywhere' );


# Check error messages
# .. for bootstrap()
eval { DynaLoader::bootstrap() };
like( $@, qr/^Usage: DynaLoader::bootstrap\(module\)/,
        "calling DynaLoader::bootstrap() with no argument" );

eval { package egg_bacon_sausage_and_spam; DynaLoader::bootstrap("egg_bacon_sausage_and_spam") };
if ($Config{usedl}) {
    like( $@, qr/^Can't locate loadable object for module egg_bacon_sausage_and_spam/,
        "calling DynaLoader::bootstrap() with a package without binary object" );
} else {
     like( $@, qr/^Can't load module egg_bacon_sausage_and_spam/,
        "calling DynaLoader::bootstrap() with a package without binary object" );
}

# .. for dl_load_file()
SKIP: {
    skip( "no dl_load_file with dl_none.xs", 2 ) unless $Config{usedl};
    eval { DynaLoader::dl_load_file() };
    like( $@, qr/^Usage: DynaLoader::dl_load_file\(filename, flags=0\)/,
            "calling DynaLoader::dl_load_file() with no argument" );

    eval { no warnings 'uninitialized'; DynaLoader::dl_load_file(undef) };
    is( $@, '', "calling DynaLoader::dl_load_file() with undefined argument" );     # is this expected ?
}

my ($dlhandle, $dlerr);
eval { $dlhandle = DynaLoader::dl_load_file("egg_bacon_sausage_and_spam") };
$dlerr = DynaLoader::dl_error();
SKIP: {
    skip( "dl_load_file() does not attempt to load file on VMS (and thus does not fail) when \@dl_require_symbols is empty", 1 ) if $^O eq 'VMS';
    ok( !$dlhandle, "calling DynaLoader::dl_load_file() without an existing library should fail" );
}
ok( defined $dlerr, "dl_error() returning an error message: '$dlerr'" );

# Checking for any particular error messages or numeric codes
# is very unportable, please do not try to do that.  A failing
# dl_load_file() is not even guaranteed to set the $! or the $^E.

# ... dl_findfile()
SKIP: {
    my @files = ();
    eval { @files = DynaLoader::dl_findfile("c") };
    is( $@, '', "calling dl_findfile()" );
    # Some platforms are known to not have a "libc"
    # (not at least by that name) that the dl_findfile()
    # could find.
    skip( "dl_findfile test not appropriate on $^O", 1 )
	if $^O =~ /(win32|vms|openbsd|bitrig|cygwin|vos|os390)/i;
    # Play safe and only try this test if this system
    # looks pretty much Unix-like.
    skip( "dl_findfile test not appropriate on $^O", 1 )
	unless -d '/usr' && -f '/bin/ls';
    skip( "dl_findfile test not always appropriate when cross-compiling", 1 )
        if $Config{usecrosscompile};
    cmp_ok( scalar @files, '>=', 1, "array should contain one result or more: libc => (@files)" );
}

# Now try to load well known XS modules
my $extensions = $Config{'dynamic_ext'};
$extensions =~ s|/|::|g;

for my $module (sort keys %modules) {
    SKIP: {
        if ($extensions !~ /\b$module\b/) {
            delete($modules{$module});
            skip( "$module not available", 3);
        }
        eval "use $module";
        is( $@, '', "loading $module" );
    }
}

# checking internal consistency
is( scalar @DynaLoader::dl_librefs, scalar keys %modules, "checking number of items in \@dl_librefs" );
is( scalar @DynaLoader::dl_modules, scalar keys %modules, "checking number of items in \@dl_modules" );

my @loaded_modules = @DynaLoader::dl_modules;
for my $libref (reverse @DynaLoader::dl_librefs) {
SKIP: {
        skip( "unloading unsupported on $^O", 2 )
            if ($old_darwin || $^O eq 'VMS');
        my $module = pop @loaded_modules;
        skip( "File::Glob sets PL_opfreehook", 2 ) if $module eq 'File::Glob';
        my $r = eval { DynaLoader::dl_unload_file($libref) };
        is( $@, '', "calling dl_unload_file() for $module" );
        is( $r,  1, " - unload was successful" );
    }
}

SKIP: {
    skip( "mod2fname not defined on this platform", 4 )
        unless defined &DynaLoader::mod2fname && $Config{d_libname_unique};

    is(
        DynaLoader::mod2fname(["Hash", "Util"]),
        "PL_Hash__Util",
        "mod2fname + libname_unique works"
    );

    is(
        DynaLoader::mod2fname([("Hash", "Util") x 25]),
        "PL_" . join("_", ("Hash", "Util")x25),
        "mod2fname + libname_unique collapses double __'s for long names"
    );

    is(
        DynaLoader::mod2fname([("Haash", "Uttil") x 25]),
        "PL_" . join("_", ("HAsh", "UTil")x25),
        "mod2fname + libname_unique collapses repeated characters for long names"
    );

    is(
        DynaLoader::mod2fname([("Hash", "Util")x30]),
        substr(("PL_" . join("_", ("Hash", "Util")x30)), 0, 255 - (length($Config::Config{dlext})+1)),
        "mod2fname + libname_unique correctly truncates long names"
    );
}

