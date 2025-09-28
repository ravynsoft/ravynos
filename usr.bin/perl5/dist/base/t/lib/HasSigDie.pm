package HasSigDie;

$SIG{__DIE__} = sub { "Die, Bart, Die!" };

1;

