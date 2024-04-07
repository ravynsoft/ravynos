#!./perl

use VMS::Filespec;
use File::Spec;

foreach (<DATA>) {
  chomp;
  s/\s*#.*//;
  next if /^\s*$/;
  push(@tests,$_);
}

require 'test.pl';
plan(tests => scalar(2*@tests)+6);

my $vms_unix_rpt;
my $vms_efs;

if ($^O eq 'VMS') {
    if (eval 'require VMS::Feature') {
        $vms_unix_rpt = VMS::Feature::current("filename_unix_report");
        $vms_efs = VMS::Feature::current("efs_charset");
    } else {
        my $unix_rpt = $ENV{'DECC$FILENAME_UNIX_REPORT'} || '';
        my $efs_charset = $ENV{'DECC$EFS_CHARSET'} || '';
        $vms_unix_rpt = $unix_rpt =~ /^[ET1]/i; 
        $vms_efs = $efs_charset =~ /^[ET1]/i; 
    }
}



foreach $test (@tests) {
  ($arg,$func,$expect2,$expect5) = split(/(?<!\\)\s+/,$test);

  $arg =~ s/\\//g; # to get whitespace into the argument escape with \
  $expect2 =~ s/\\//g;
  $expect5 =~ s/\\//g;
  $expect2 = undef if $expect2 eq 'undef';
  $expect2 = undef if $expect2 eq '^';
  $expect5 = undef if $expect5 eq 'undef';
  $expect5 = $expect2 if $expect5 eq '^';

  if ($vms_efs) {
	$expect = $expect5;
  }
  else {
	$expect = $expect2;
  }

  $rslt = eval "$func('$arg')";
  is($@, '', "eval ${func}('$arg')");
  if ($expect ne '^*') {
    is(lc($rslt), lc($expect), "${func}('$arg'): '$rslt'");
  }
  else {
    is(lc($rslt), lc($expect), "${func}('$arg'): '$rslt' # TODO fix ODS-5 test");
  }
}

$defwarn = <<'EOW';
# Note: This failure may have occurred because your default device
# was set using a non-concealed logical name.  If this is the case,
# you will need to determine by inspection that the two resultant
# file specifications shown above are in fact equivalent.
EOW

is(uc(rmsexpand('[]')),   "\U$ENV{DEFAULT}", 'rmsexpand()') || print $defwarn;
is(lc(rmsexpand('from.here')),"\L$ENV{DEFAULT}from.here") || print $defwarn;
is(lc(rmsexpand('from')),     "\L$ENV{DEFAULT}from")      || print $defwarn;

is(lc(rmsexpand('from.here','cant:[get.there];2')),
   'cant:[get.there]from.here;2')                     || print $defwarn;


# Make sure we're using redirected mkdir, which strips trailing '/', since
# the CRTL's mkdir can't handle this.
ok(mkdir('testdir/',0777),      'using redirected mkdir()');
ok(rmdir('testdir/'),           '    rmdir()');

__DATA__

# Column definitions:
#
#  Column 1: Argument (path spec to be transformed)
#  Column 2: Function that is to do the transformation
#  Column 3: Expected result when DECC$EFS_CHARSET is not in effect
#  Column 4: Expected result when DECC$EFS_CHARSET is in effect
#            ^ means expect same result for EFS as for non-EFS
#            ^* means TODO when EFS is in effect

# lots of underscores used to minimize collision with existing logical names

# Basic VMS to Unix filespecs
__some_:[__where_.__over_]__the_.__rainbow_    unixify /__some_/__where_/__over_/__the_.__rainbow_ ^
__some_:<__where_.__over_>__the_.__rainbow_    unixify /__some_/__where_/__over_/__the_.__rainbow_ ^
[.__some_.__where_.__over_]__the_.__rainbow_   unixify __some_/__where_/__over_/__the_.__rainbow_ ^
[-.__some_.__where_.__over_]__the_.__rainbow_  unixify ../__some_/__where_/__over_/__the_.__rainbow_ ^
[.__some_.--.__where_.__over_]__the_.__rainbow_        unixify __some_/../../__where_/__over_/__the_.__rainbow_ ^
[.__some_...__where_.__over_]__the_.__rainbow_ unixify __some_/.../__where_/__over_/__the_.__rainbow_ ^
[...__some_.__where_.__over_]__the_.__rainbow_ unixify .../__some_/__where_/__over_/__the_.__rainbow_ ^
[.__some_.__where_.__over_...]__the_.__rainbow_        unixify __some_/__where_/__over_/.../__the_.__rainbow_ ^
[.__some_.__where_.__over_...] unixify __some_/__where_/__over_/.../ ^
[.__some_.__where_.__over_.-]  unixify __some_/__where_/__over_/../ ^
[]	unixify		./	^
[-]	unixify		../	^
[--]	unixify		../../	^
[...]	unixify		.../	^
__lyrics_:[__are_.__very_^.__sappy_]__but_^.__rhymes_^.__are_.__true_    unixify   /__lyrics_/__are_/__very_.__sappy_/__but_.__rhymes_.__are_.__true_ ^
[.$(macro)]	unixify	$(macro)/ ^
^+foo.tmp	unixify +foo.tmp ^
[-.foo^_^_bar]	unixify ../foo\ \ bar/ ^
[]foo.tmp	unixify	./foo.tmp ^

# and back again
/__some_/__where_/__over_/__the_.__rainbow_    vmsify  __some_:[__where_.__over_]__the_.__rainbow_ ^
__some_/__where_/__over_/__the_.__rainbow_     vmsify  [.__some_.__where_.__over_]__the_.__rainbow_ ^
../__some_/__where_/__over_/__the_.__rainbow_  vmsify  [-.__some_.__where_.__over_]__the_.__rainbow_ ^
__some_/../../__where_/__over_/__the_.__rainbow_       vmsify  [.__some_.--.__where_.__over_]__the_.__rainbow_ ^
.../__some_/__where_/__over_/__the_.__rainbow_ vmsify  [...__some_.__where_.__over_]__the_.__rainbow_ ^
__some_/.../__where_/__over_/__the_.__rainbow_ vmsify  [.__some_...__where_.__over_]__the_.__rainbow_  ^
/__some_/.../__where_/__over_/__the_.__rainbow_        vmsify  __some_:[...__where_.__over_]__the_.__rainbow_ ^
__some_/__where_.DIR;1                         vmsify  [.__some_]__where_.DIR;1 ^
__some_/_;_where_.DIR;1                        vmsify  [.__some_]_^;_where_.DIR;1 ^
__some_/__where_/...   vmsify  [.__some_.__where_...] ^
/__where_/...  vmsify  __where_:[...] ^
.	vmsify	[]	^
..	vmsify	[-]	^
../..	vmsify	[--]	^
.../	vmsify	[...]	^
/	vmsify	sys$disk:[000000] ^
./$(macro)/	vmsify	[.$(macro)] ^
./$(macro)	vmsify	[]$(macro) ^
./$(m+	vmsify	[]$^(m^+	^
foo-bar-0^.01/	vmsify [.foo-bar-0_01] [.foo-bar-0^.01]
\ foo.tmp	vmsify ^_foo.tmp ^
+foo.tmp	vmsify ^+foo.tmp ^
../foo\ \ bar/	vmsify [-.foo^_^_bar] ^
./foo.tmp	vmsify []foo.tmp ^
x/r*???????	vmsify [.x]r*??????? ^

# Fileifying directory specs
__down_:[__the_.__garden_.__path_]     fileify __down_:[__the_.__garden_]__path_.dir;1 ^
[.__down_.__the_.__garden_.__path_]    fileify [.__down_.__the_.__garden_]__path_.dir;1 ^
/__down_/__the_/__garden_/__path_      fileify /__down_/__the_/__garden_/__path_.dir;1 ^
/__down_/__the_/__garden_/__path_/     fileify /__down_/__the_/__garden_/__path_.dir;1 ^
__down_/__the_/__garden_/__path_       fileify __down_/__the_/__garden_/__path_.dir;1 ^
__down_:[__the_.__garden_]__path_      fileify __down_:[__the_.__garden_]__path_.dir;1 ^
__down_:[__the_.__garden_]__path_.     fileify ^ __down_:[__the_.__garden_]__path_^..dir;1 # N.B. trailing . ==> null type
__down_:[__the_]__garden_.__path_      fileify ^ __down_:[__the_]__garden_^.__path_.dir;1 #undef
/__down_/__the_/__garden_/__path_.     fileify ^ /__down_/__the_/__garden_/__path_..dir;1 # N.B. trailing . ==> null type
/__down_/__the_/__garden_.__path_      fileify ^ /__down_/__the_/__garden_.__path_.dir;1
__down_::__the_:[__garden_.__path_]    fileify __down_::__the_:[__garden_]__path_.dir;1 ^
__down_::__the_:[__garden_]            fileify __down_::__the_:[000000]__garden_.dir;1 ^

# and pathifying them
__down_:[__the_.__garden_]__path_.dir;1        pathify __down_:[__the_.__garden_.__path_] ^
[.__down_.__the_.__garden_]__path_.dir pathify [.__down_.__the_.__garden_.__path_] ^
/__down_/__the_/__garden_/__path_.dir  pathify /__down_/__the_/__garden_/__path_/ ^
__down_/__the_/__garden_/__path_.dir   pathify __down_/__the_/__garden_/__path_/ ^
__down_:[__the_.__garden_]__path_      pathify __down_:[__the_.__garden_.__path_] ^
__down_:[__the_.__garden_]__path_.     pathify ^ __down_:[__the_.__garden_.__path_^.] # N.B. trailing . ==> null type
__down_:[__the_]__garden_.__path_      pathify ^ __down_:[__the_.__garden_^.__path_] # undef
/__down_/__the_/__garden_/__path_.     pathify /__down_/__the_/__garden_/__path__/ /__down_/__the_/__garden_/__path_./ # N.B. trailing . ==> null type
/__down_/__the_/__garden_.__path_      pathify /__down_/__the_/__garden____path_/ /__down_/__the_/__garden_.__path_/
__down_:[__the_.__garden_]__path_.dir;2        pathify ^ #N.B. ;2
__path_        pathify __path_/ ^
/__down_/__the_/__garden_/.    pathify /__down_/__the_/__garden_/./ ^
/__down_/__the_/__garden_/..   pathify /__down_/__the_/__garden_/../ ^
/__down_/__the_/__garden_/...  pathify /__down_/__the_/__garden_/.../ ^ 
__path_.notdir pathify __path__notdir/ __path_.notdir/

# Both VMS/Unix and file/path conversions
__down_:[__the_.__garden_]__path_.dir;1        unixpath        /__down_/__the_/__garden_/__path_/ ^
/__down_/__the_/__garden_/__path_      vmspath __down_:[__the_.__garden_.__path_] ^
__down_:[__the_.__garden_.__path_]     unixpath        /__down_/__the_/__garden_/__path_/ ^
__down_:[__the_.__garden_.__path_...]  unixpath        /__down_/__the_/__garden_/__path_/.../ ^
/__down_/__the_/__garden_/__path_.dir  vmspath __down_:[__the_.__garden_.__path_] ^
[.__down_.__the_.__garden_]__path_.dir unixpath        __down_/__the_/__garden_/__path_/ ^
__down_/__the_/__garden_/__path_       vmspath [.__down_.__the_.__garden_.__path_] ^
__path_        vmspath [.__path_] ^
/	vmspath	sys$disk:[000000] ^
/sys$scratch	vmspath	sys$scratch: ^

# Redundant characters in Unix paths
//__some_/__where_//__over_/../__the_.__rainbow_       vmsify  __some_:[__where_.__over_.-]__the_.__rainbow_ ^
/__some_/__where_//__over_/./__the_.__rainbow_ vmsify  __some_:[__where_.__over_]__the_.__rainbow_ ^
..//../	vmspath	[--] ^
./././	vmspath	[] ^
./../.	vmsify	[-] ^

# Our override of File::Spec->canonpath can do some strange things
__dev:[__dir.000000]__foo     File::Spec->canonpath   __dev:[__dir.000000]__foo ^
__dev:[__dir.][000000]__foo   File::Spec->canonpath   __dev:[__dir]__foo ^
