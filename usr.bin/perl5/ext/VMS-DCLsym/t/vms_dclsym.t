print "1..30\n";

require VMS::DCLsym or die "not ok 1\n";
print "ok 1\n";

tie %syms, VMS::DCLsym or die "not ok 2\n";
print "ok 2\n";

$name = 'FOO_'.time();
$syms{$name} = 'Perl_test';
print +($! ? "#(\$! = $!)\nnot " : ''),"ok 3\n";

print +($syms{$name} eq 'Perl_test' ? '' : 'not '),"ok 4\n";

($val) = `Show Symbol $name` =~ /(\w+)"$/;
print +($val eq 'Perl_test' ? '' : 'not '),"ok 5\n";

while (($sym,$val) = each %syms) {
  last if $sym eq $name && $val eq 'Perl_test';
}
print +($sym ? '' : 'not '),"ok 6\n";

delete $syms{$name};
print +($! ? "#(\$! = $!)\nnot " : ''),"ok 7\n";

print +(defined($syms{$name}) ? 'not ' : ''),"ok 8\n";

$obj = new VMS::DCLsym 'GLOBAL';
print +($obj ? '' : 'not '),"ok 9\n";

print +($obj->clearcache(0) ? '' : 'not '),"ok 10\n";
print +($obj->clearcache(1) ? 'not ' : ''),"ok 11\n";

print +($obj->setsym($name,'Another_test') ? '' : 'not '),"ok 12\n";

($val,$tab) = $obj->getsym($name);
print +($val eq 'Another_test' && $tab eq 'GLOBAL' ? '' : 'not '),"ok 13\n";

print +($obj->delsym($name,'LOCAL') ? 'not ' : ''),"ok 14\n";
print +($obj->delsym($name,'GLOBAL') ? '' : 'not '),"ok 15\n";

($val,$tab) = $obj->getsym($name);
print +(defined($val) || defined($tab) ? 'not ' : ''),"ok 16\n";

($val) = `Show Symbol/Global $name` =~ /==\s+"(\w+)"$/;
print +(defined($val) ? 'not ' : ''),"ok 17\n";

tie %gsyms, VMS::DCLsym, 'GLOBAL' or die "not ok 18\n";
print "ok 18\n";

print +(tied(%gsyms) =~ /^VMS::DCLsym/ ? '' : 'not '),"ok 19\n";
print +(exists $gsyms{$name} ? 'not ' : ''),"ok 20\n";

$gsyms{$name} = 'Perl_test';
print +($! ? "#(\$! = $!)\nnot " : ''),"ok 21\n";

print +($gsyms{$name} eq 'Perl_test' ? '' : 'not '),"ok 22\n";

($val) = `Show Symbol/Global $name` =~ /==\s+"(\w+)"$/;
print +($val eq 'Perl_test' ? '' : 'not '),"ok 23\n";

delete $gsyms{$name};
print +($! ? "#(\$! = $!)\nnot " : ''),"ok 24\n";

($val,$tab) = $obj->getsym($name);
print +(defined($val) || defined($tab) ? 'not ' : ''),"ok 25\n";

($val) = `Show Symbol/Global $name` =~ /==\s+"(\w+)"$/;
print +($val eq 'Perl_test' ? 'not ' : ''),"ok 26\n";

print +($syms{':LOCAL'} ?  '' : 'not '),"ok 27\n";
print +($syms{':GLOBAL'} ? 'not ' : ''),"ok 28\n";

print +($gsyms{':LOCAL'} ?  'not ' : ''),"ok 29\n";
print +($gsyms{':GLOBAL'} ? '' : 'not '),"ok 30\n";
