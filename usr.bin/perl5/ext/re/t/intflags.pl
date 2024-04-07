use re 'Debug' => qw(DUMP FLAGS);
our $count;
my $code= '(?{$count++})';
my @p= (
    qr/(foo)(?1)?/,
    qr/\Gfoo/,
    qr/.*foo/,
    qr/^foo/,
    qr/(foo(*THEN)bar|food)/,
    qr/a.*b.*/,
    qr/a{1,4}\Gfoo/,
    qr/a+/,
    do { use re 'eval'; qr/a$code/},
);

print STDERR "-OK-\n";
