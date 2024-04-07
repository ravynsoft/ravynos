#!perl

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc( qw(../lib) );
}

use strict;
use warnings;

plan(tests => 73);


# Dedupe @INC. In a future patch we /may/ refuse to process items
# more than once and deduping here will prevent the tests from failing
# should we make that change.
my %seen; @INC = grep {!$seen{$_}++} @INC;

my $nonfile = tempfile();

# The tests for ' ' and '.h' never did fail, but previously the error reporting
# code would read memory before the start of the SV's buffer

for my $file ($nonfile, ' ') {
    eval {
	require $file;
    };

    like $@, qr/^Can't locate $file in \@INC \(\@INC[\w ]+: \Q@INC\E\) at/,
	"correct error message for require '$file'";
}

# Check that the "(you may need to install..) hint is included in the
# error message where (and only where) appropriate.
#
# Basically the hint should be issued for any filename where converting
# back from Foo/Bar.pm to Foo::Bar gives you a legal bare word which could
# follow "require" in source code.

{

    # may be any letter of an identifier
    my $I = "\x{393}";  # "\N{GREEK CAPITAL LETTER GAMMA}"
    # Continuation char: may only be 2nd+ letter of an identifier
    my $C = "\x{387}";  # "\N{GREEK ANO TELEIA}"

    for my $test_data (
        # thing to require        pathname in err mesg     err includes hint?
        [ "No::Such::Module1",          "No/Such/Module1.pm",       1 ],
        [ "'No/Such/Module1.pm'",       "No/Such/Module1.pm",       1 ],
        [ "_No::Such::Module1",         "_No/Such/Module1.pm",      1 ],
        [ "'_No/Such/Module1.pm'",      "_No/Such/Module1.pm",      1 ],
        [ "'No/Such./Module.pm'",       "No/Such./Module.pm",       0 ],
        [ "No::1Such::Module",          "No/1Such/Module.pm",       1 ],
        [ "'No/1Such/Module.pm'",       "No/1Such/Module.pm",       1 ],
        [ "1No::Such::Module",           undef,                     0 ],
        [ "'1No/Such/Module.pm'",       "1No/Such/Module.pm",       0 ],

        # utf8 variants
        [ "No::Such${I}::Module1",      "No/Such${I}/Module1.pm",   1 ],
        [ "'No/Such${I}/Module1.pm'",   "No/Such${I}/Module1.pm",   1 ],
        [ "_No::Such${I}::Module1",     "_No/Such${I}/Module1.pm",  1 ],
        [ "'_No/Such${I}/Module1.pm'",  "_No/Such${I}/Module1.pm",  1 ],
        [ "'No/Such${I}./Module.pm'",   "No/Such${I}./Module.pm",   0 ],
        [ "No::1Such${I}::Module",      "No/1Such${I}/Module.pm",   1 ],
        [ "'No/1Such${I}/Module.pm'",   "No/1Such${I}/Module.pm",   1 ],
        [ "1No::Such${I}::Module",       undef,                     0 ],
        [ "'1No/Such${I}/Module.pm'",   "1No/Such${I}/Module.pm",   0 ],

        # utf8 with continuation char in 1st position
        [ "No::${C}Such::Module1",      undef,                      0 ],
        [ "'No/${C}Such/Module1.pm'",   "No/${C}Such/Module1.pm",   0 ],
        [ "_No::${C}Such::Module1",     undef,                      0 ],
        [ "'_No/${C}Such/Module1.pm'",  "_No/${C}Such/Module1.pm",  0 ],
        [ "'No/${C}Such./Module.pm'",   "No/${C}Such./Module.pm",   0 ],
        [ "No::${C}1Such::Module",      undef,                      0 ],
        [ "'No/${C}1Such/Module.pm'",   "No/${C}1Such/Module.pm",   0 ],
        [ "1No::${C}Such::Module",      undef,                      0 ],
        [ "'1No/${C}Such/Module.pm'",   "1No/${C}Such/Module.pm",   0 ],

    ) {
        my ($require_arg, $err_path, $has_hint) = @$test_data;

        my $exp;
        if (defined $err_path) {
            $exp = "Can't locate $err_path in \@INC";
            if ($has_hint) {
                my $hint = $err_path;
                $hint =~ s{/}{::}g;
                $hint =~ s/\.pm$//;
                $exp .= " (you may need to install the $hint module)";
            }
            $exp .= " (\@INC entries checked: @INC) at";
        }
        else {
            # undef implies a require which doesn't compile,
            # rather than one which triggers a run-time error.
            # We'll set exp to a suitable value later;
            $exp = "";
        }

        my $err;
        {
            no warnings qw(syntax utf8);
            if ($require_arg =~ /[^\x00-\xff]/) {
                eval "require $require_arg";
                $err = $@;
                utf8::decode($err);
            }
            else {
                eval "require $require_arg";
                $err = $@;
            }
        }

        for ($err, $exp, $require_arg) {
            s/([^\x00-\xff])/sprintf"\\x{%x}",ord($1)/ge;
        }
        if (length $exp) {
            $exp = qr/^\Q$exp\E/;
        }
        else {
            $exp = qr/syntax error at|Unrecognized character/;
        }
        like $err, $exp,
                "err for require $require_arg";
    }
}



eval "require ::$nonfile";

like $@, qr/^Bareword in require must not start with a double-colon:/,
        "correct error message for require ::$nonfile";

eval {
    require "$nonfile.ph";
};

like $@, qr/^Can't locate $nonfile\.ph in \@INC \(did you run h2ph\?\) \(\@INC[\w ]+: @INC\) at/;

for my $file ("$nonfile.h", ".h") {
    eval {
	require $file
    };

    like $@, qr/^Can't locate \Q$file\E in \@INC \(change \.h to \.ph maybe\?\) \(did you run h2ph\?\) \(\@INC[\w ]+: @INC\) at/,
	"correct error message for require '$file'";
}

for my $file ("$nonfile.ph", ".ph") {
    eval {
	require $file
    };

    like $@, qr/^Can't locate \Q$file\E in \@INC \(did you run h2ph\?\) \(\@INC[\w ]+: @INC\) at/,
	"correct error message for require '$file'";
}

eval 'require <foom>';
like $@, qr/^<> at require-statement should be quotes at /, 'require <> error';

my $module   = tempfile();
my $mod_file = "$module.pm";

open my $module_fh, ">", $mod_file or die $!;
print { $module_fh } "print 1; 1;\n";
close $module_fh;

chmod 0333, $mod_file;

SKIP: {
    skip_if_miniperl("these modules may not be available to miniperl", 2);

    push @INC, '../lib';
    require Cwd;
    require File::Spec::Functions;
    if ($^O eq 'cygwin') {
        require Win32;
    }

    # Going to try to switch away from root.  Might not work.
    # (stolen from t/op/stat.t)
    my $olduid = $>;
    eval { $> = 1; };
    skip "Can't test permissions meaningfully if you're superuser", 2
        if ($^O eq 'cygwin' ? Win32::IsAdminUser() : $> == 0);

    local @INC = ".";
    eval "use $module";
    like $@,
        qr<^\QCan't locate $mod_file:>,
        "special error message if the file exists but can't be opened";

    SKIP: {
        skip "Can't make the path absolute", 1
            if !defined(Cwd::getcwd());

        my $file = File::Spec::Functions::catfile(Cwd::getcwd(), $mod_file);
        eval {
            require($file);
        };
        like $@,
            qr<^\QCan't locate $file:>,
            "...even if we use a full path";
    }

    # switch uid back (may not be implemented)
    eval { $> = $olduid; };
}

1 while unlink $mod_file;

# I can't see how to test the EMFILE case
# I can't see how to test the case of not displaying @INC in the message.
# (and does that only happen on VMS?)

# fail and print the full filename
eval { no warnings 'syscalls'; require "strict.pm\0invalid"; };
like $@, qr/^Can't locate strict\.pm\\0invalid: /, 'require nul check [perl #117265]';
{
  my $WARN;
  local $SIG{__WARN__} = sub { $WARN = shift };
  {
    my $ret = do "strict.pm\0invalid";
    my $exc = $@;
    my $err = $!;
    is $ret, undef, 'do nulstring returns undef';
    is $exc, '',    'do nulstring clears $@';
    $! = $err;
    ok $!{ENOENT},  'do nulstring fails with ENOENT';
    like $WARN, qr{^Invalid \\0 character in pathname for do: strict\.pm\\0invalid at }, 'do nulstring warning';
  }

  $WARN = '';
  eval { require "strict.pm\0invalid"; };
  like $WARN, qr{^Invalid \\0 character in pathname for require: strict\.pm\\0invalid at }, 'nul warning';
  like $@, qr{^Can't locate strict\.pm\\0invalid: }, 'nul error';

  $WARN = '';
  local @INC = @INC;
  set_up_inc( "lib\0invalid" );
  eval { require "unknown.pm" };
  like $WARN, qr{^Invalid \\0 character in \@INC entry for require: lib\\0invalid at }, 'nul warning';
}
eval "require strict\0::invalid;";
like $@, qr/^syntax error at \(eval \d+\) line 1/, 'parse error with \0 in barewords module names';

# Refs and globs that stringify with embedded nulls
# These crashed from 5.20 to 5.24 [perl #128182].
eval { no warnings 'syscalls'; require eval "qr/\0/" };
like $@, qr/^Can't locate \(\?\^:\\0\):/,
    'require ref that stringifies with embedded null';
eval { no strict; no warnings 'syscalls'; require *{"\0a"} };
like $@, qr/^Can't locate \*main::\\0a:/,
    'require ref that stringifies with embedded null';

eval { require undef };
like $@, qr/^Missing or undefined argument to require /;

eval { do undef };
like $@, qr/^Missing or undefined argument to do /;

eval { require "" };
like $@, qr/^Missing or undefined argument to require /;

eval { do "" };
like $@, qr/^Missing or undefined argument to do /;

# non-searchable pathnames shouldn't mention @INC in the error

my $nonsearch = "./no_such_file.pm";

eval "require \"$nonsearch\"";

like $@, qr/^Can't locate \Q$nonsearch\E at/,
        "correct error message for require $nonsearch";

{
    # make sure require doesn't treat a non-PL_sv_undef undef as
    # success in %INC
    # GH #17428
    push @INC, "lib";
    ok(!eval { require CannotParse; }, "should fail to load");
    local %INC = %INC; # copies \&PL_sv_undef into a new undef
    ok(!eval { require CannotParse; },
       "check the second attempt also fails");
    like $@, qr/Attempt to reload/, "check we failed for the right reason";
}

{
    fresh_perl_like(
        'unshift @INC, sub { sub { 0 } }; require "asdasd";',
        qr/asdasd did not return a true value/,
        { }, '@INC hook blocks do not cause segfault');
}

{
    # make sure that modifications to %INC during an INC hooks lifetime
    # don't result in us having an empty string for the cop_file.
    # Older perls will output "error at  line 1".

    fresh_perl_like(
        'use lib qq(./lib); BEGIN{ unshift @INC, '
       .'sub { if ($_[1] eq "CannotParse.pm" and !$seen++) { '
       .'eval q(require $_[1]); warn $@; my $code= qq[die qq(error)];'
       .'open my $fh,"<", q(lib/Dies.pm); return $fh } } } require CannotParse;',
        qr!\Asyntax error.*?^error at /loader/0x[A-Fa-f0-9]+/CannotParse\.pm line 1\.!ms,
        { }, 'Inc hooks have the correct cop_file');
}
{
    # this can segfault or assert prior to @INC hardening.
    fresh_perl_like(
        'unshift @INC, sub { *INC=["a","b"] }; '
       .'eval "require Frobnitz" or print $@',
        qr!\(\@INC[\w ]+: CODE\(0x[A-Fa-f0-9]+\) b\)!,
        { }, 'INC hooks do not segfault when overwritten');
}
{
    # this is the defined behavior, but in older perls the error message
    # would lie and say "contains: a b", which is true in the sense that
    # it is the value of @INC after the require, but not the directory
    # list that was looked at.
    fresh_perl_like(
        '@INC = (sub { @INC=("a","b"); () }, "z"); '
       .'eval "require Frobnitz" or print $@',
        qr!\(\@INC[\w ]+: CODE\(0x[A-Fa-f0-9]+\) b\)!,
        { }, 'INC hooks that overwrite @INC continue as expected (skips a and z)');
}
{
    # as of 5.37.7
    fresh_perl_like(
        '@INC = (sub { @INC=qw(a b); undef $INC }, "z"); '
       .'eval "require Frobnitz" or print $@',
        qr!\(\@INC[\w ]+: CODE\(0x[A-Fa-f0-9]+\) a b\)!,
        { }, 'INC hooks that overwrite @INC and undef $INC continue at start');
}
{
    # as of 5.37.7
    fresh_perl_like(
        'sub CB::INCDIR { return "b", "c","d" }; '
       .'@INC = ("a",bless({},"CB"),"e");'
       .'eval "require Frobnitz" or print $@',
        qr!\(\@INC[\w ]+: a CB=HASH\(0x[A-Fa-f0-9]+\) b c d e\)!,
        { }, 'INCDIR works as expected');
}
{
    # as of 5.37.7
    fresh_perl_like(
        '@INC = ("a",bless({},"CB"),"e");'
       .'eval "require Frobnitz" or print $@',
        qr!Can't locate object method "INC", nor "INCDIR" nor string overload via package "CB" in object hook in \@INC!,
        { }, 'Objects with no INC or INCDIR method and no overload throw an error');
}
{
    # as of 5.37.7
    fresh_perl_like(
        'package CB { use overload q("") => sub { "Fnorble" };} @INC = ("a",bless({},"CB"),"e");'
       .'eval "require Frobnitz" or print $@',
        qr!\(\@INC[\w ]+: a Fnorble e\)!,
        { }, 'Objects with no INC or INCDIR method but with an overload are stringified');
}
{
    # as of 5.37.7
    fresh_perl_like(
        'package CB { use overload q(0+) => sub { 12345 }, fallback=>1;} @INC = ("a",bless({},"CB"),"e");'
       .'eval "require Frobnitz" or print $@',
        qr!\(\@INC[\w ]+: a 12345 e\)!,
        { }, 'Objects with no INC or INCDIR method but with an overload with fallback are stringified');
}
{
    # as of 5.37.7
    fresh_perl_like(
        '{package CB; use overload qw("")=>sub { "blorg"};} '
       .'@INC = ("a",bless({},"CB"),"e");'
       .'eval "require Frobnitz" or print $@',
        qr!\(\@INC[\w ]+: a blorg e\)!,
        { }, 'Objects with overload and no INC or INCDIR method are stringified');
}
{
    # as of 5.37.7
    fresh_perl_like(
        '@INC = ("a",bless(sub { warn "blessed sub called" },"CB"),"e");'
       .'eval "require Frobnitz" or print $@',
        qr!blessed sub called.*\(\@INC[\w ]+: a CB=CODE\(0x[a-fA-F0-9]+\) e\)!s,
        { }, 'Blessed subs with no hook methods are executed');
}
{
    # as of 5.37.7
    fresh_perl_like(
        '@INC = ("a",bless(sub { die "blessed sub called" },"CB"),"e");'
       .'eval "require Frobnitz" or print $@',
        qr!INC sub hook died--halting \@INC search!s,
        { }, 'Blessed subs that die produce expected extra message');
}
{
    # as of 5.37.7
    fresh_perl_like(
        'sub CB::INC { die "bad mojo" } '
       .'@INC = ("a",bless(sub { die "blessed sub called" },"CB"),"e");'
       .'eval "require Frobnitz" or print $@',
        qr!bad mojo.*INC method hook died--halting \@INC search!s,
        { }, 'Blessed subs with methods call method and produce expected message');
}
{
    # as of 5.37.7
    fresh_perl_like(
        '@INC = ("a",[bless([],"CB"),1],"e");'
       .'eval "require Frobnitz" or print $@',
        qr!Can't locate object method "INC", nor "INCDIR" nor string overload via package "CB" in object in ARRAY hook in \@INC!s,
        { }, 'Blessed objects with no hook methods in array form produce expected exception');
}
{
    # as of 5.37.7
    fresh_perl_like(
        'sub CB::INCDIR { "i" } sub CB2::INCDIR { }'
       .'@INC = ("a",bless(sub{"b"},"CB"),bless(sub{"c"},"CB2"),"e");'
       .'eval "require Frobnitz" or print $@',
        qr!\(\@INC[\w ]+: a CB=CODE\(0x[a-fA-F0-9]+\) i CB2=CODE\(0x[a-fA-F0-9]+\) e\)!s,
        { }, 'Blessed subs with INCDIR methods call INCDIR');
}
{
    # as of 5.37.7
    fresh_perl_like(
        'sub CB::INCDIR { return @{$_[2]} }'
       .'@INC = ("a",[bless([],"CB"),"b"],"c");'
       .'eval "require Frobnitz" or print $@',
        qr!\(\@INC[\w ]+: a ARRAY\(0x[a-fA-F0-9]+\) CB=ARRAY\(0x[a-fA-F0-9]+\) b c\)!s,
        { }, 'INCDIR ref returns are stringified');
}
