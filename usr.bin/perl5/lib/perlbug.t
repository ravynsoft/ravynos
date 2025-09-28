#!./perl
use strict;

# test that perlbug generates somewhat sane reports, but don't
# actually send them

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

require './test.pl';

# lifted from perl5db.t
my $extracted_program = '../utils/perlbug'; # unix, nt, ...
if ($^O eq 'VMS') { $extracted_program = '[-.utils]perlbug.com'; }
if (!(-e $extracted_program)) {
    print "1..0 # Skip: $extracted_program was not built\n";
    exit 0;
}

my $result;
my $testreport = 'test.rep';
unlink $testreport;

sub _slurp {
        my $file = shift;
        ok(-f $file, "saved report $file exists");
        open(F, '<', $file) or return undef;
        local $/;
        my $ret = <F>;
        close F;
        $ret;
}

sub _dump {
        my $file = shift;
        my $contents = shift;
        open(F, '>', $file) or return;
        print F $contents;
        close F;
        return 1;
}

plan(25);


# check -d
$result = runperl( progfile => $extracted_program,
                   args     => ['-d'] );
like($result, qr/Site configuration information/,
     'config information dumped with -d');


# check -v
$result = runperl( progfile => $extracted_program,
                   args     => ['-d', '-v'] );
like($result, qr/Complete configuration data/,
     'full config information dumped with -d -v');

# check that we need -t
$result = runperl( progfile => $extracted_program,
                   stderr   => 1, # perlbug dies with "\n";
                   stdin    => undef);
like($result, qr/Please use perlbug interactively./,
     'checks for terminal in non-test mode');


# test -okay (mostly noninteractive)
$result = runperl( progfile => $extracted_program,
                   args     => ['-okay', '-F', $testreport] );
like($result, qr/Report saved/, 'build report saved');
like(_slurp($testreport), qr/Perl reported to build OK on this system/,
     'build report looks sane');
unlink $testreport;


# test -nokay (a bit more interactive)
$result = runperl( progfile => $extracted_program,
                   stdin    => 'f', # save to File
                   args     => ['-t',
                                '-nokay',
                                '-e', 'file',
                                '-F', $testreport] );
like($result, qr/Report saved/, 'build failure report saved');
like(_slurp($testreport), qr/This is a build failure report for perl/,
     'build failure report looks sane');
unlink $testreport;


# test a regular report
$result = runperl( progfile => $extracted_program,
                   # no CLI options for these
                   stdin    => "\n" # Module
                             . "\n" # Category
                             . "\n" # Severity
                             . "\n" # Editor
                             . "f", # save to File
                   args     => ['-t',
                                # runperl has trouble with whitespace
                                '-s', "testingperlbug",
                                '-r', 'username@example.com',
                                '-c', 'none',
                                '-b', 'testreportbody',
                                '-e', 'file',
                                '-F', $testreport] );
like($result, qr/Report saved/, 'fake bug report saved');
my $contents = _slurp($testreport);
like($contents, qr/Subject: testingperlbug/,
     'Subject included in fake bug report');
like($contents, qr/testreportbody/, 'body included in fake bug report');
unlink $testreport;


# test wrapping of long lines
my $body = 'body.txt';
unlink $body;
my $A = 'A'x9;
ok(_dump($body, ("$A "x120)), 'wrote 1200-char body to file');

my $attachment = 'attached.txt';
unlink $attachment;
my $B = 'B'x9;
ok(_dump($attachment, ("$B "x120)), 'wrote 1200-char attachment to file');

$result = runperl( progfile => $extracted_program,
                   stdin    => "testing perlbug\n" # Subject
                             . "\n" # Module
                             . "\n" # Category
                             . "\n" # Severity
                             . "f", # save to File
                   args     => ['-t',
                                '-r', 'username@example.com',
                                '-c', 'none',
                                '-f', $body,
                                '-p', $attachment,
                                '-e', 'file',
                                '-F', $testreport] );
like($result, qr/Report saved/, 'fake bug report saved');
my $contents = _slurp($testreport);
unlink $testreport, $body, $attachment;
like($contents, qr/Subject: testing perlbug/,
     'Subject included in fake bug report');
like($contents, qr/$A/, 'body included in fake bug report');
like($contents, qr/$B/, 'attachment included in fake bug report');

my $maxlen1 = 0; # body
my $maxlen2 = 0; # attachment
for (split(/\n/, $contents)) {
        my $len = length;
        # content lines setting path-like environment variables like PATH, PERLBREW_PATH, MANPATH,...
        #  will start "\s*xxxxPATH=" where "xxx" is zero or more non white space characters. These lines can
        #  easily get over 1000 characters (see ok-test below) with no internal spaces, so they
        #  will not get wrapped at white space.
        # See also https://github.com/perl/perl5/issues/15544 for more information
        $maxlen1 = $len if $len > $maxlen1 and !/(?:$B|^\s*\S*PATH=)/;
        $maxlen2 = $len if $len > $maxlen2 and  /$B/;
}
ok($maxlen1 < 1000, "[perl #128020] long body lines are wrapped: maxlen $maxlen1");
ok($maxlen2 > 1000, "long attachment lines are not wrapped: maxlen $maxlen2");

$result = runperl( progfile => $extracted_program, stderr => 1, args => ['-o'] ); # Invalid option
like($result, qr/^\s*This program is designed/, "No leading error messages with help from invalid arg.");

$result = runperl( progfile => $extracted_program, stderr => 1, args => ['--help'] ); # Invalid option
like($result, qr/^\s*perlbug version \d+\.\d+\n+This program is designed/, "No leading error messages with help from --help and version is displayed.");

$result = runperl( progfile => $extracted_program, stderr => 1, args => ['--version'] ); # Invalid option
like($result, qr/^perlbug version \d+\.\d+\n$/, "No leading error messages with --version");
#print $result;
