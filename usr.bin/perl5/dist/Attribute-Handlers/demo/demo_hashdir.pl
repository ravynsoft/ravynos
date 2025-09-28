use Attribute::Handlers autotie => { Dir => 'Tie::Dir qw(DIR_UNLINK)' };

my %dot : Dir('.', DIR_UNLINK);

print join "\n", keys %dot;

delete $dot{killme};

print join "\n", keys %dot;
