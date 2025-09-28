#!perl -w
use strict;

# creates perllibst.h file for inclusion from perllib.c

use Config;

my @statics = split /\s+/, $Config{static_ext};
open my $fh, '>', 'perllibst.h' or die "Failed to write to perllibst.h:$!";

my @statics1 = map {local $_=$_;s/\//__/g;$_} @statics;
my @statics2 = map {local $_=$_;s/\//::/g;$_} @statics;
print $fh "/*DO NOT EDIT\n  this file is included from perllib.c to init static extensions */\n";
print $fh "#ifdef STATIC1\n",(map {"    \"$_\",\n"} @statics),"#undef STATIC1\n#endif\n";
print $fh "#ifdef STATIC2\n",(map {"    EXTERN_C void boot_$_ (pTHX_ CV* cv);\n"} @statics1),"#undef STATIC2\n#endif\n";
print $fh "#ifdef STATIC3\n",(map {"    newXS(\"$statics2[$_]::bootstrap\", boot_$statics1[$_], file);\n"} 0 .. $#statics),"#undef STATIC3\n#endif\n";
close $fh;
