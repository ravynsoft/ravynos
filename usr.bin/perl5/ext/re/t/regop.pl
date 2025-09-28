use re Debug=>qw(DUMP EXECUTE TRIEC TEST);
my @tests=(
  XY     =>  'X(A|[B]Q||C|D)Y' ,
  foobar =>  '[f][o][o][b][a][r]',
  x  =>  '.[XY].',
  'ABCD' => '(?:ABCP|ABCG|ABCE|ABCB|ABCA|ABCD)',
  'D:\\dev/perl/ver/28321_/perl.exe'=>
  '/(\\.COM|\\.EXE|\\.BAT|\\.CMD|\\.VBS|\\.VBE|\\.JS|\\.JSE|\\.WSF|\\.WSH|\\.pyo|\\.pyc|\\.pyw|\\.py)$/i',
  'q'=>'[q]',
  "path_sep:\t8490" => '^(\\S{1,9}):\\s*(\\d+)$',
  '' => '(?(DEFINE)(?<foo>foo))(?(DEFINE)(?<bar>(?&foo)bar))(?(DEFINE)(?<baz>(?&bar)baz))(?(DEFINE)(?<bop>(?&baz)bop))',
);
while (@tests) {
    my ($str,$pat)=splice @tests,0,2;
    warn "\n";
    $pat="/$pat/" if substr($pat,0,1) ne '/';
    # string eval to get the free regex message in the right place.
    eval qq[
        warn "$str"=~$pat ? "%MATCHED%" : "%FAILED%","\n";
    ];
    die $@ if $@;
}
