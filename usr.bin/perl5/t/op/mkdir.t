#!./perl -w

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc('../lib');
}

plan tests => 17;

unless (eval {
    require File::Path;
    File::Path::rmtree('blurfl') if -d 'blurfl';
    1
}) {
    diag("$0 may fail if its temporary directory remains from a previous run");
    diag("Attempted to load File::Path to delete directory t/blurfl - error was\n$@");
    diag("\nIf you have problems, please manually delete t/blurfl");
}    

# tests 3 and 7 rather naughtily expect English error messages
$ENV{'LC_ALL'} = 'C';
$ENV{LANGUAGE} = 'C'; # GNU locale extension

sub errno_or_skip {
    SKIP: {
	if (is_miniperl && !eval { local $!; require Errno }) {
	    skip "Errno not built yet", 1;
	}
	eval "ok($_[0])";
    }
}

ok(mkdir('blurfl',0777));
ok(!mkdir('blurfl',0777));
errno_or_skip('$!{EEXIST} || $! =~ /cannot move|exist|denied|unknown/i');
ok(-d 'blurfl');
ok(rmdir('blurfl'));
ok(!rmdir('blurfl'));
errno_or_skip('
    $!{ENOENT}
       || $! =~ /cannot find|such|exist|not found|not a directory|unknown/i
');
ok(mkdir('blurfl'));
ok(rmdir('blurfl'));

# trailing slashes will be removed before the system call to mkdir
ok(mkdir('blurfl///'));
ok(-d 'blurfl');
ok(rmdir('blurfl///'));
ok(!-d 'blurfl');

# test default argument

$_ = 'blurfl';
ok(mkdir);
ok(-d);
ok(rmdir);
ok(!-d);
$_ = 'lfrulb';
