#!/usr/bin/perl

use strict;
use Test::More tests => 30;
use Config;
use DynaLoader;
use ExtUtils::CBuilder;
use lib (-d 't' ? File::Spec->catdir(qw(t lib)) : 'lib');
use PrimitiveCapture;

my ($source_file, $obj_file, $lib_file);

require_ok( 'ExtUtils::ParseXS' );

chdir('t') if -d 't';
push @INC, '.';

$ExtUtils::ParseXS::DIE_ON_ERROR = 1;
$ExtUtils::ParseXS::AUTHOR_WARNINGS = 1;

use Carp; #$SIG{__WARN__} = \&Carp::cluck;

# The linker on some platforms doesn't like loading libraries using relative
# paths. Android won't find relative paths, and system perl on macOS will
# refuse to load relative paths. The path that DynaLoader uses to load the
# .so or .bundle file is based on the @INC path that the library is loaded
# from. The XSTest module we're using for testing is in the current directory,
# so we need an absolute path in @INC rather than '.'. Just convert all of the
# paths to absolute for simplicity.
@INC = map { File::Spec->rel2abs($_) } @INC;

#########################

{ # first block: try without linenumbers
my $pxs = ExtUtils::ParseXS->new;
# Try sending to filehandle
tie *FH, 'Foo';
$pxs->process_file( filename => 'XSTest.xs', output => \*FH, prototypes => 1 );
like tied(*FH)->content, '/is_even/', "Test that output contains some text";

$source_file = 'XSTest.c';

# Try sending to file
$pxs->process_file(filename => 'XSTest.xs', output => $source_file, prototypes => 0);
ok -e $source_file, "Create an output file";

my $quiet = $ENV{PERL_CORE} && !$ENV{HARNESS_ACTIVE};
my $b = ExtUtils::CBuilder->new(quiet => $quiet);

SKIP: {
  skip "no compiler available", 2
    if ! $b->have_compiler;
  $obj_file = $b->compile( source => $source_file );
  ok $obj_file, "ExtUtils::CBuilder::compile() returned true value";
  ok -e $obj_file, "Make sure $obj_file exists";
}

SKIP: {
  skip "no dynamic loading", 5
    if !$b->have_compiler || !$Config{usedl};
  my $module = 'XSTest';
  $lib_file = $b->link( objects => $obj_file, module_name => $module );
  ok $lib_file, "ExtUtils::CBuilder::link() returned true value";
  ok -e $lib_file,  "Make sure $lib_file exists";

  eval {require XSTest};
  is $@, '', "No error message recorded, as expected";
  ok  XSTest::is_even(8),
    "Function created thru XS returned expected true value";
  ok !XSTest::is_even(9),
    "Function created thru XS returned expected false value";

  # Win32 needs to close the DLL before it can unlink it, but unfortunately
  # dl_unload_file was missing on Win32 prior to perl change #24679!
  if ($^O eq 'MSWin32' and defined &DynaLoader::dl_unload_file) {
    for (my $i = 0; $i < @DynaLoader::dl_modules; $i++) {
      if ($DynaLoader::dl_modules[$i] eq $module) {
        DynaLoader::dl_unload_file($DynaLoader::dl_librefs[$i]);
        last;
      }
    }
  }
}

my $seen = 0;
open my $IN, '<', $source_file
  or die "Unable to open $source_file: $!";
while (my $l = <$IN>) {
  $seen++ if $l =~ m/#line\s1\s/;
}
is( $seen, 1, "Line numbers created in output file, as intended" );
{
    #rewind .c file and regexp it to look for code generation problems
    local $/ = undef;
    seek($IN, 0, 0);
    my $filecontents = <$IN>;
    $filecontents =~ s/^#if defined\(__HP_cc\).*\n#.*\n#endif\n//gm;
    my $good_T_BOOL_re =
qr|\QXS_EUPXS(XS_XSTest_T_BOOL)\E
.+?
#line \d+\Q "XSTest.c"
	ST(0) = boolSV(RETVAL);
    }
    XSRETURN(1);
}
\E|s;
    like($filecontents, $good_T_BOOL_re, "T_BOOL doesn\'t have an extra sv_newmortal or sv_2mortal");

    my $good_T_BOOL_2_re =
qr|\QXS_EUPXS(XS_XSTest_T_BOOL_2)\E
.+?
#line \d+\Q "XSTest.c"
	sv_setsv(ST(0), boolSV(in));
	SvSETMAGIC(ST(0));
    }
    XSRETURN(1);
}
\E|s;
    like($filecontents, $good_T_BOOL_2_re, 'T_BOOL_2 doesn\'t have an extra sv_newmortal or sv_2mortal');
    my $good_T_BOOL_OUT_re =
qr|\QXS_EUPXS(XS_XSTest_T_BOOL_OUT)\E
.+?
#line \d+\Q "XSTest.c"
	sv_setsv(ST(0), boolSV(out));
	SvSETMAGIC(ST(0));
    }
    XSRETURN_EMPTY;
}
\E|s;
    like($filecontents, $good_T_BOOL_OUT_re, 'T_BOOL_OUT doesn\'t have an extra sv_newmortal or sv_2mortal');

}
close $IN or die "Unable to close $source_file: $!";

unless ($ENV{PERL_NO_CLEANUP}) {
  for ( $obj_file, $lib_file, $source_file) {
    next unless defined $_;
    1 while unlink $_;
  }
}
}

#####################################################################

{ # second block: try with linenumbers
my $pxs = ExtUtils::ParseXS->new;
# Try sending to filehandle
tie *FH, 'Foo';
$pxs->process_file(
    filename => 'XSTest.xs',
    output => \*FH,
    prototypes => 1,
    linenumbers => 0,
);
like tied(*FH)->content, '/is_even/', "Test that output contains some text";

$source_file = 'XSTest.c';

# Try sending to file
$pxs->process_file(
    filename => 'XSTest.xs',
    output => $source_file,
    prototypes => 0,
    linenumbers => 0,
);
ok -e $source_file, "Create an output file";


my $seen = 0;
open my $IN, '<', $source_file
  or die "Unable to open $source_file: $!";
while (my $l = <$IN>) {
  $seen++ if $l =~ m/#line\s1\s/;
}
close $IN or die "Unable to close $source_file: $!";
is( $seen, 0, "No linenumbers created in output file, as intended" );

unless ($ENV{PERL_NO_CLEANUP}) {
  for ( $obj_file, $lib_file, $source_file) {
    next unless defined $_;
    1 while unlink $_;
  }
}
}
#####################################################################

{ # third block: broken typemap
my $pxs = ExtUtils::ParseXS->new;
tie *FH, 'Foo';
my $stderr = PrimitiveCapture::capture_stderr(sub {
  $pxs->process_file(filename => 'XSBroken.xs', output => \*FH);
});
like $stderr, '/No INPUT definition/', "Exercise typemap error";
}
#####################################################################

{ # fourth block: https://github.com/Perl/perl5/issues/19661
  my $pxs = ExtUtils::ParseXS->new;
  tie *FH, 'Foo';
  my ($stderr, $filename);
  {
    $filename = 'XSFalsePositive.xs';
    $stderr = PrimitiveCapture::capture_stderr(sub {
      $pxs->process_file(filename => $filename, output => \*FH, prototypes => 1);
    });
    TODO: {
      local $TODO = 'GH 19661';
      unlike $stderr,
        qr/Warning: duplicate function definition 'do' detected in \Q$filename\E/,
        "No 'duplicate function definition' warning observed in $filename";
    }
  }
  {
    $filename = 'XSFalsePositive2.xs';
    $stderr = PrimitiveCapture::capture_stderr(sub {
      $pxs->process_file(filename => $filename, output => \*FH, prototypes => 1);
    });
    TODO: {
      local $TODO = 'GH 19661';
      unlike $stderr,
        qr/Warning: duplicate function definition 'do' detected in \Q$filename\E/,
        "No 'duplicate function definition' warning observed in $filename";
      }
  }
}

#####################################################################

{ # tight cpp directives
  my $pxs = ExtUtils::ParseXS->new;
  tie *FH, 'Foo';
  my $stderr = PrimitiveCapture::capture_stderr(sub { eval {
    $pxs->process_file(
      filename => 'XSTightDirectives.xs',
      output => \*FH,
      prototypes => 1);
  } or warn $@ });
  my $content = tied(*FH)->{buf};
  my $count = 0;
  $count++ while $content=~/^XS_EUPXS\(XS_My_do\)\n\{/mg;
  is $stderr, undef, "No error expected from TightDirectives.xs";
  is $count, 2, "Saw XS_MY_do definition the expected number of times";
}

{ # Alias check
  my $pxs = ExtUtils::ParseXS->new;
  tie *FH, 'Foo';
  my $stderr = PrimitiveCapture::capture_stderr(sub {
    $pxs->process_file(
      filename => 'XSAlias.xs',
      output => \*FH,
      prototypes => 1);
  });
  my $content = tied(*FH)->{buf};
  my $count = 0;
  $count++ while $content=~/^XS_EUPXS\(XS_My_do\)\n\{/mg;
  is $stderr,
    "Warning: Aliases 'pox' and 'dox', 'lox' have"
    . " identical values of 1 in XSAlias.xs, line 9\n"
    . "    (If this is deliberate use a symbolic alias instead.)\n"
    . "Warning: Conflicting duplicate alias 'pox' changes"
    . " definition from '1' to '2' in XSAlias.xs, line 10\n"
    . "Warning: Aliases 'docks' and 'dox', 'lox' have"
    . " identical values of 1 in XSAlias.xs, line 11\n"
    . "Warning: Aliases 'xunx' and 'do' have identical values"
    . " of 0 - the base function in XSAlias.xs, line 13\n",
    "Saw expected warnings from XSAlias.xs in AUTHOR_WARNINGS mode";

  my $expect = quotemeta(<<'EOF_CONTENT');
         cv = newXSproto_portable("My::dachs", XS_My_do, file, "$");
         XSANY.any_i32 = 1;
         cv = newXSproto_portable("My::do", XS_My_do, file, "$");
         XSANY.any_i32 = 0;
         cv = newXSproto_portable("My::docks", XS_My_do, file, "$");
         XSANY.any_i32 = 1;
         cv = newXSproto_portable("My::dox", XS_My_do, file, "$");
         XSANY.any_i32 = 1;
         cv = newXSproto_portable("My::lox", XS_My_do, file, "$");
         XSANY.any_i32 = 1;
         cv = newXSproto_portable("My::pox", XS_My_do, file, "$");
         XSANY.any_i32 = 2;
         cv = newXSproto_portable("My::xukes", XS_My_do, file, "$");
         XSANY.any_i32 = 0;
         cv = newXSproto_portable("My::xunx", XS_My_do, file, "$");
         XSANY.any_i32 = 0;
EOF_CONTENT
  $expect=~s/(?:\\[ ])+/\\s+/g;
  $expect=qr/$expect/;
  like $content, $expect, "Saw expected alias initialization";

  #diag $content;
}
{ # Alias check with no dev warnings.
  my $pxs = ExtUtils::ParseXS->new;
  tie *FH, 'Foo';
  my $stderr = PrimitiveCapture::capture_stderr(sub {
    $pxs->process_file(
      filename => 'XSAlias.xs',
      output => \*FH,
      prototypes => 1,
      author_warnings => 0);
  });
  my $content = tied(*FH)->{buf};
  my $count = 0;
  $count++ while $content=~/^XS_EUPXS\(XS_My_do\)\n\{/mg;
  is $stderr,
    "Warning: Conflicting duplicate alias 'pox' changes"
    . " definition from '1' to '2' in XSAlias.xs, line 10\n",
    "Saw expected warnings from XSAlias.xs";

  my $expect = quotemeta(<<'EOF_CONTENT');
         cv = newXSproto_portable("My::dachs", XS_My_do, file, "$");
         XSANY.any_i32 = 1;
         cv = newXSproto_portable("My::do", XS_My_do, file, "$");
         XSANY.any_i32 = 0;
         cv = newXSproto_portable("My::docks", XS_My_do, file, "$");
         XSANY.any_i32 = 1;
         cv = newXSproto_portable("My::dox", XS_My_do, file, "$");
         XSANY.any_i32 = 1;
         cv = newXSproto_portable("My::lox", XS_My_do, file, "$");
         XSANY.any_i32 = 1;
         cv = newXSproto_portable("My::pox", XS_My_do, file, "$");
         XSANY.any_i32 = 2;
         cv = newXSproto_portable("My::xukes", XS_My_do, file, "$");
         XSANY.any_i32 = 0;
         cv = newXSproto_portable("My::xunx", XS_My_do, file, "$");
         XSANY.any_i32 = 0;
EOF_CONTENT
  $expect=~s/(?:\\[ ])+/\\s+/g;
  $expect=qr/$expect/;
  like $content, $expect, "Saw expected alias initialization";

  #diag $content;
}
{
    my $file = $INC{"ExtUtils/ParseXS.pm"};
    $file=~s!ExtUtils/ParseXS\.pm\z!perlxs.pod!;
    open my $fh, "<", $file
        or die "Failed to open '$file' for read:$!";
    my $pod_version = "";
    while (defined(my $line= readline($fh))) {
        if ($line=~/\(also known as C<xsubpp>\)\s+(\d+\.\d+)/) {
            $pod_version = $1;
            last;
        }
    }
    close $fh;
    ok($pod_version, "Found the version from perlxs.pod");
    is($pod_version, $ExtUtils::ParseXS::VERSION,
        "The version in perlxs.pod should match the version of ExtUtils::ParseXS");
}

{
    my $pxs = ExtUtils::ParseXS->new;
    tie *FH, 'Foo';
    my $exception;
    my $stderr = PrimitiveCapture::capture_stderr(sub {
        eval {
            $pxs->process_file(
                filename => "XSNoMap.xs",
                output => \*FH,
               );
            1;
        } or $exception = $@;
    });
    is($stderr, undef, "should fail to parse");
    like($exception, qr/Could not find a typemap for C type 'S \*'/,
         "check we throw rather than trying to deref '2'");
}

#####################################################################

sub Foo::TIEHANDLE { bless {}, 'Foo' }
sub Foo::PRINT { shift->{buf} .= join '', @_ }
sub Foo::content { shift->{buf} }
