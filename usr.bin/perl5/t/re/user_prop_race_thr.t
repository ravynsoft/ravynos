#!perl
use strict;
use warnings;

require './test.pl';
skip_all_without_config('useithreads');
skip_all_if_miniperl("no dynamic loading on miniperl, no threads");

plan(3);

require threads;

{
    fresh_perl_is('
        use threads;
        use strict;
        use warnings;

        sub main::IsA {
            use feature "state";

            state $upper_char = ord "A";
            state $lower_char = ord "a";

            return sprintf "%x", $lower_char++ if shift;
            return sprintf "%x", $upper_char++;
        }

        my @threads = map +threads->create(sub {
            sleep 0.1;

            for (1..2500) {
                return 0 unless eval "qq(A) =~ qr/\\\p{main::IsA}/";
                return 0 unless eval "qq(a) =~ qr/\\\p{main::IsA}/i";
            }

            return 1;
        }), (0..1);
        my $success  = $threads[0]->join;
           $success += $threads[1]->join;
        print $success;',
    2,
    {},
    "Simultaneous threads worked");

}

{
    fresh_perl_is('
        use threads;
        use strict;
        use warnings;

        sub InLongSleep {
            use feature "state";

            state $which = 0;

            sleep(60) unless $which++;
            return "0042";
        }

        sub InQuick {
            return sprintf "%x", ord("C");
        }

        my $thread0 = threads->create(sub {

            my $a = \'\p{InLongSleep}\';
            qr/$a/;

            return 1;
        });
        my $thread1 = threads->create(sub {
            sleep 1;

            my $c = \'\p{InQuick}\';
            return "C" =~ /$c/;
        });
        print $thread1->join;
        $thread0->detach();',
    1,
    {},
    "One thread hung on a defn doesn't impinge on other's other defns");
}

{
    fresh_perl_like('
        use threads;
        use strict;
        use warnings;

        sub InLongSleep {
            use feature "state";

            state $which = 0;

            sleep(25) unless $which++;
            return "0042";
        }

        my @threads = map +threads->create(sub {
            sleep 1;

            my $a = \'\p{InLongSleep}\';
            qr/$a/;

            return 1;
        }), (0..1);
        $threads[1]->join;
        $threads[0]->detach();',
    qr/Thread \d+ terminated abnormally: Timeout waiting for another thread to define "InLongSleep" in regex/,
    {},
    "One thread hung on a definition doesn't delay another indefinitely");
}

1;
