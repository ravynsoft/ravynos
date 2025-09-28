#!./perl

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
    require './test.pl';
}

{ # perl #116190
  fresh_perl_is('print qq!@F!', '1 2',
		{
		 stdin => "1:2",
		 switches => [ '-n', '-F:' ],
		}, "passing -F implies -a");
  fresh_perl_is('print qq!@F!', '1 2',
		{
		 stdin => "1:2",
		 switches => [ '-F:' ],
		}, "passing -F implies -an");
  fresh_perl_is('print join q!,!, @F', '1,2',
		{
		 stdin => "1 2",
		 switches => [ '-a' ],
		}, "passing -a implies -n");
}


my $have_config = eval { require Config; 1 };
SKIP:
{
  $have_config or skip "Can't check if we have threads", 1;
  $Config::Config{usethreads} or skip "No threads", 1;
  is_miniperl() and skip "threads module not available under miniperl", 1;
  # this would only fail under valgrind/ASAN
  fresh_perl_is('print $F[1]; threads->new(sub {})->join', "b",
                {
                    switches => [ "-F,", "-Mthreads" ],
                    stdin => "a,b,c",
                }, "PL_splitstr freed in each thread");
}

{
  # old value of PL_splitstr wasn't freed with multiple switches (it wasn't safe to before)
  # this would only fail under valgrind/LSAN
  fresh_perl_is('print $F[1]', "b",
                {
                    switches => [ "-F:", "-F," ],
                    stdin => "a,b,c",
                }, "PL_splitstr freed on extra -F switch");
}

done_testing();
