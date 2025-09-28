BEGIN {
    unshift @INC, 't/lib/';
}
BEGIN { print "1..1\n" }

use Filter::Simple::ExportTest 'ok';

notok 1;
