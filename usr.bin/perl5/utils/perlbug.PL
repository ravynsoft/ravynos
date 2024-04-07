#!/usr/local/bin/perl

use strict;
use warnings;

use Config;
use File::Basename qw(&basename &dirname);
use Cwd;
use File::Spec::Functions;

# List explicitly here the variables you want Configure to
# generate.  Metaconfig only looks for shell variables, so you
# have to mention them as if they were shell variables, not
# %Config entries.  Thus you write
#  $startperl
# to ensure Configure will look for $Config{startperl}.
#  $perlpath

# This forces PL files to create target in same directory as PL file.
# This is so that make depend always knows where to find PL derivatives.
my $origdir = cwd;
chdir dirname($0);
my $file = basename($0, '.PL');
$file .= '.com' if $^O eq 'VMS';

open OUT, ">", $file or die "Can't create $file: $!";

# get patchlevel.h timestamp

-e catfile(updir, "patchlevel.h")
    or die "Can't find patchlevel.h: $!";

my $patchlevel_date = (stat _)[9];

# TO DO (perhaps): store/embed $Config::config_sh into perlbug. When perlbug is
# used, compare $Config::config_sh with the stored version. If they differ then
# append a list of individual differences to the bug report.


print "Extracting $file (with variable substitutions)\n";

# In this section, perl variables will be expanded during extraction.
# You can use $Config{...} to use Configure variables.

my $extract_version = sprintf("%vd", $^V);

print OUT <<"!GROK!THIS!";
$Config{startperl}
    eval 'exec $Config{perlpath} -S \$0 \${1+"\$@"}'
	if 0; # ^ Run only under a shell

my \$config_tag1 = '$extract_version - $Config{cf_time}';

my \$patchlevel_date = $patchlevel_date;
!GROK!THIS!

# In the following, perl variables are not expanded during extraction.

print OUT <<'!NO!SUBS!';
my @patches = Config::local_patches();
my $patch_tags = join "", map /(\S+)/ ? "+$1 " : (), @patches;

BEGIN { pop @INC if $INC[-1] eq '.' }
use warnings;
use strict;
use Config;
use File::Spec;		# keep perlbug Perl 5.005 compatible
use Getopt::Std;
use File::Basename 'basename';

$Getopt::Std::STANDARD_HELP_VERSION = 1;

sub paraprint;

BEGIN {
    eval { require Mail::Send;};
    $::HaveSend = ($@ eq "");
    eval { require Mail::Util; } ;
    $::HaveUtil = ($@ eq "");
    # use secure tempfiles wherever possible
    eval { require File::Temp; };
    $::HaveTemp = ($@ eq "");
    eval { require Module::CoreList; };
    $::HaveCoreList = ($@ eq "");
    eval { require Text::Wrap; };
    $::HaveWrap = ($@ eq "");
};

our $VERSION = "1.43";

#TODO:
#       make sure failure (transmission-wise) of Mail::Send is accounted for.
#       (This may work now. Unsure of the original author's issue -JESSE 2008-06-08)
#       - Test -b option

my( $file, $usefile, $cc, $address, $thanksaddress,
    $filename, $messageid, $domain, $subject, $from, $verbose, $ed, $outfile,
    $fh, $me, $body, $andcc, %REP, $ok, $thanks, $progname,
    $Is_MSWin32, $Is_Linux, $Is_VMS, $Is_OpenBSD,
    $report_about_module, $category, $severity,
    %opt, $have_attachment, $attachments, $has_patch, $mime_boundary
);

my $running_noninteractively = !-t STDIN;

my $perl_version = $^V ? sprintf("%vd", $^V) : $];

my $config_tag2 = "$perl_version - $Config{cf_time}";

Init();

if ($opt{h}) { Help(); exit; }
if ($opt{d}) { Dump(*STDOUT); exit; }
if ($running_noninteractively && !$opt{t} && !($ok and not $opt{n})) {
    paraprint <<"EOF";
Please use $progname interactively. If you want to
include a file, you can use the -f switch.
EOF
    die "\n";
}

Query();
Edit() unless $usefile || ($ok and not $opt{n});
NowWhat();
if ($address) {
    Send();
    if ($thanks) {
	print "\nThank you for taking the time to send a thank-you message!\n\n";

	paraprint <<EOF
Please note that mailing lists are moderated, your message may take a while to
show up.
EOF
    } else {
	print "\nThank you for taking the time to file a bug report!\n\n";

	paraprint <<EOF
Please note that mailing lists are moderated, your message may take a while to
show up. Please consider submitting your report directly to the issue tracker
at https://github.com/Perl/perl5/issues
EOF
    }

} else {
    save_message_to_disk($outfile);
}

exit;

sub ask_for_alternatives { # (category|severity)
    my $name = shift;
    my %alts = (
	'category' => {
	    'default' => 'core',
	    'ok'      => 'install',
	    # Inevitably some of these will end up in RT whatever we do:
	    'thanks'  => 'thanks',
	    'opts'    => [qw(core docs install library utilities)], # patch, notabug
	},
	'severity' => {
	    'default' => 'low',
	    'ok'      => 'none',
	    'thanks'  => 'none',
	    'opts'    => [qw(critical high medium low wishlist none)], # zero
	},
    );
    die "Invalid alternative ($name) requested\n" unless grep(/^$name$/, keys %alts);
    my $alt = "";
    my $what = $ok || $thanks;
    if ($what) {
	$alt = $alts{$name}{$what};
    } else {
 	my @alts = @{$alts{$name}{'opts'}};
    print "\n\n";
	paraprint <<EOF;
Please pick a $name from the following list:

    @alts
EOF
	my $err = 0;
	do {
	    if ($err++ > 5) {
		die "Invalid $name: aborting.\n";
	    }
        $alt = _prompt('', "\u$name", $alts{$name}{'default'});
		$alt ||= $alts{$name}{'default'};
	} while !((($alt) = grep(/^$alt/i, @alts)));
    }
    lc $alt;
}

sub HELP_MESSAGE { Help(); exit; }
sub VERSION_MESSAGE { print "perlbug version $VERSION\n"; }

sub Init {
    # -------- Setup --------

    $Is_MSWin32 = $^O eq 'MSWin32';
    $Is_VMS = $^O eq 'VMS';
    $Is_Linux = lc($^O) eq 'linux';
    $Is_OpenBSD = lc($^O) eq 'openbsd';

    # Thanks address
    $thanksaddress = 'perl-thanks@perl.org';

    # Defaults if getopts fails.
    $outfile = (basename($0) =~ /^perlthanks/i) ? "perlthanks.rep" : "perlbug.rep";
    $cc = $::Config{'perladmin'} || $::Config{'cf_email'} || $::Config{'cf_by'} || '';

    HELP_MESSAGE() unless getopts("Adhva:s:b:f:F:r:e:SCc:to:n:T:p:", \%opt);

    # This comment is needed to notify metaconfig that we are
    # using the $perladmin, $cf_by, and $cf_time definitions.
    # -------- Configuration ---------

    if (basename ($0) =~ /^perlthanks/i) {
	# invoked as perlthanks
	$opt{T} = 1;
	$opt{C} = 1; # don't send a copy to the local admin
    }

    if ($opt{T}) {
	$thanks = 'thanks';
    }
    
    $progname = $thanks ? 'perlthanks' : 'perlbug';
    # Target address
    $address = $opt{a} || ($thanks ? $thanksaddress : "");

    # Users address, used in message and in From and Reply-To headers
    $from = $opt{r} || "";

    # Include verbose configuration information
    $verbose = $opt{v} || 0;

    # Subject of bug-report message
    $subject = $opt{s} || "";

    # Send a file
    $usefile = ($opt{f} || 0);

    # File to send as report
    $file = $opt{f} || "";

    # We have one or more attachments
    $have_attachment = ($opt{p} || 0);
    $mime_boundary = ('-' x 12) . "$VERSION.perlbug" if $have_attachment;

    # Comma-separated list of attachments
    $attachments = $opt{p} || "";
    $has_patch = 0; # TBD based on file type

    for my $attachment (split /\s*,\s*/, $attachments) {
        unless (-f $attachment && -r $attachment) {
            die "The attachment $attachment is not a readable file: $!\n";
        }
        $has_patch = 1 if $attachment =~ m/\.(patch|diff)$/;
    }

    # File to output to
    $outfile = $opt{F} || "$progname.rep";

    # Body of report
    $body = $opt{b} || "";
	
    # Editor
    $ed = $opt{e} || $ENV{VISUAL} || $ENV{EDITOR} || $ENV{EDIT}
	|| ($Is_VMS && "edit/tpu")
	|| ($Is_MSWin32 && "notepad")
	|| "vi";

    # Not OK - provide build failure template by finessing OK report
    if ($opt{n}) {
	if (substr($opt{n}, 0, 2) eq 'ok' )	{
	    $opt{o} = substr($opt{n}, 1);
	} else {
	    Help();
	    exit();
	}
    }

    # OK - send "OK" report for build on this system
    $ok = '';
    if ($opt{o}) {
	if ($opt{o} eq 'k' or $opt{o} eq 'kay') {
	    my $age = time - $patchlevel_date;
	    if ($opt{o} eq 'k' and $age > 60 * 24 * 60 * 60 ) {
		my $date = localtime $patchlevel_date;
		print <<"EOF";
"perlbug -ok" and "perlbug -nok" do not report on Perl versions which
are more than 60 days old.  This Perl version was constructed on
$date.  If you really want to report this, use
"perlbug -okay" or "perlbug -nokay".
EOF
		exit();
	    }
	    # force these options
	    unless ($opt{n}) {
		$opt{S} = 1; # don't prompt for send
		$opt{b} = 1; # we have a body
		$body = "Perl reported to build OK on this system.\n";
	    }
	    $opt{C} = 1; # don't send a copy to the local admin
	    $opt{s} = 1; # we have a subject line
	    $subject = ($opt{n} ? 'Not ' : '')
		    . "OK: perl $perl_version ${patch_tags}on"
		    ." $::Config{'archname'} $::Config{'osvers'} $subject";
	    $ok = 'ok';
	} else {
	    Help();
	    exit();
	}
    }

    # Possible administrator addresses, in order of confidence
    # (Note that cf_email is not mentioned to metaconfig, since
    # we don't really want it. We'll just take it if we have to.)
    #
    # This has to be after the $ok stuff above because of the way
    # that $opt{C} is forced.
    $cc = $opt{C} ? "" : (
	$opt{c} || $::Config{'perladmin'}
	|| $::Config{'cf_email'} || $::Config{'cf_by'}
    );

    if ($::HaveUtil) {
		$domain = Mail::Util::maildomain();
    } elsif ($Is_MSWin32) {
		$domain = $ENV{'USERDOMAIN'};
    } else {
		require Sys::Hostname;
		$domain = Sys::Hostname::hostname();
    }

    # Message-Id - rjsf
    $messageid = "<$::Config{'version'}_${$}_".time."\@$domain>"; 

    # My username
    $me = $Is_MSWin32 ? $ENV{'USERNAME'}
	    : $^O eq 'os2' ? $ENV{'USER'} || $ENV{'LOGNAME'}
	    : eval { getpwuid($<) };	# May be missing

    $from = $::Config{'cf_email'}
       if !$from && $::Config{'cf_email'} && $::Config{'cf_by'} && $me &&
               ($me eq $::Config{'cf_by'});
} # sub Init

sub Query {
    # Explain what perlbug is
    unless ($ok) {
	if ($thanks) {
	    paraprint <<'EOF';
This program provides an easy way to send a thank-you message back to the
authors and maintainers of perl.

If you wish to generate a bug report, please run it without the -T flag
(or run the program perlbug rather than perlthanks)
EOF
	} else {
	    paraprint <<"EOF";
This program provides an easy way to generate a bug report for the core
perl distribution (along with tests or patches).  To send a thank-you
note to $thanksaddress instead of a bug report, please run 'perlthanks'.

The GitHub issue tracker at https://github.com/Perl/perl5/issues is the
best place to submit your report so it can be tracked and resolved.

Please do not use $0 to report bugs in perl modules from CPAN.

Suggestions for how to find help using Perl can be found at
https://perldoc.perl.org/perlcommunity.html
EOF
	}
    }

    # Prompt for subject of message, if needed
    
    if ($subject && TrivialSubject($subject)) {
	$subject = '';
    }

    unless ($subject) {
	    print 
"First of all, please provide a subject for the report.\n";
	if ( not $thanks)  {
	    paraprint <<EOF;
This should be a concise description of your bug or problem
which will help the volunteers working to improve perl to categorize
and resolve the issue.  Be as specific and descriptive as
you can. A subject like "perl bug" or "perl problem" will make it
much less likely that your issue gets the attention it deserves.
EOF
	}

	my $err = 0;
	do {
        $subject = _prompt('','Subject');
	    if ($err++ == 5) {
		if ($thanks) {
		    $subject = 'Thanks for Perl';
		} else {
		    die "Aborting.\n";
		}
	    }
	} while (TrivialSubject($subject));
    }
    $subject = '[PATCH] ' . $subject
        if $has_patch && ($subject !~ m/^\[PATCH/i);

    # Prompt for return address, if needed
    unless ($opt{r}) {
	# Try and guess return address
	my $guess;

	$guess = $ENV{'REPLY-TO'} || $ENV{'REPLYTO'} || $ENV{'EMAIL'}
	    || $from || '';

	unless ($guess) {
		# move $domain to where we can use it elsewhere	
        if ($domain) {
		if ($Is_VMS && !$::Config{'d_socket'}) {
		    $guess = "$domain\:\:$me";
		} else {
		    $guess = "$me\@$domain" if $domain;
		}
	    }
	}

	if ($guess) {
	    unless ($ok) {
		paraprint <<EOF;
Perl's developers may need your email address to contact you for
further information about your issue or to inform you when it is
resolved.  If the default shown is not your email address, please
correct it.
EOF
	    }
	} else {
	    paraprint <<EOF;
Please enter your full internet email address so that Perl's
developers can contact you with questions about your issue or to
inform you that it has been resolved.
EOF
	}

	if ($ok && $guess) {
	    # use it
	    $from = $guess;
	} else {
	    # verify it
        $from = _prompt('','Your address',$guess);
	    $from = $guess if $from eq '';
	}
    }

    if ($from eq $cc or $me eq $cc) {
	# Try not to copy ourselves
	$cc = "yourself";
    }

    # Prompt for administrator address, unless an override was given
    if( $address and !$opt{C} and !$opt{c} ) {
	my $description =  <<EOF;
$0 can send a copy of this report to your local perl
administrator.  If the address below is wrong, please correct it,
or enter 'none' or 'yourself' to not send a copy.
EOF
	my $entry = _prompt($description, "Local perl administrator", $cc);

	if ($entry ne "") {
	    $cc = $entry;
	    $cc = '' if $me eq $cc;
	}
    }

    $cc = '' if $cc =~ /^(none|yourself|me|myself|ourselves)$/i;
    if ($cc) { 
        $andcc = " and $cc" 
    } else {
        $andcc = ''
    }

    # Prompt for editor, if no override is given
editor:
    unless ($opt{e} || $opt{f} || $opt{b}) {

    my $description;

	chomp (my $common_end = <<"EOF");
You will probably want to use a text editor to enter the body of
your report. If "$ed" is the editor you want to use, then just press
Enter, otherwise type in the name of the editor you would like to
use.

If you have already composed the body of your report, you may enter
"file", and $0 will prompt you to enter the name of the file
containing your report.
EOF

	if ($thanks) {
	    $description = <<"EOF";
It's now time to compose your thank-you message.

Some information about your local perl configuration will automatically
be included at the end of your message, because we're curious about
the different ways that people build and use perl. If you'd rather
not share this information, you're welcome to delete it.

$common_end
EOF
	} else {
	    $description =  <<"EOF";
It's now time to compose your bug report. Try to make the report
concise but descriptive. Please include any detail which you think
might be relevant or might help the volunteers working to improve
perl. If you are reporting something that does not work as you think
it should, please try to include examples of the actual result and of
what you expected.

Some information about your local perl configuration will automatically
be included at the end of your report. If you are using an unusual
version of perl, it would be useful if you could confirm that you
can replicate the problem on a standard build of perl as well.

$common_end
EOF
	}

    my $entry = _prompt($description, "Editor", $ed);
	$usefile = 0;
	if ($entry eq "file") {
	    $usefile = 1;
	} elsif ($entry ne "") {
	    $ed = $entry;
	}
    }
    if ($::HaveCoreList && !$ok && !$thanks) {
	my $description =  <<EOF;
If your bug is about a Perl module rather than a core language
feature, please enter its name here. If it's not, just hit Enter
to skip this question.
EOF

    my $entry = '';
	while ($entry eq '') {
        $entry = _prompt($description, 'Module');
	    my $first_release = Module::CoreList->first_release($entry);
	    if ($entry and not $first_release) {
		paraprint <<EOF;
$entry is not a "core" Perl module. Please check that you entered
its name correctly. If it is correct, quit this program, try searching
for $entry on https://rt.cpan.org, and report your issue there.
EOF

            $entry = '';
	} elsif (my $bug_tracker = $Module::CoreList::bug_tracker{$entry}) {
		paraprint <<"EOF";
$entry included with core Perl is copied directly from the CPAN distribution.
Please report bugs in $entry directly to its maintainers using $bug_tracker
EOF
            $entry = '';
        } elsif ($entry) {
	        $category ||= 'library';
	        $report_about_module = $entry;
            last;
        } else {
            last;
        }
	}
    }

    # Prompt for category of bug
    $category ||= ask_for_alternatives('category');

    # Prompt for severity of bug
    $severity ||= ask_for_alternatives('severity');

    # Generate scratch file to edit report in
    $filename = filename();

    # Prompt for file to read report from, if needed
    if ($usefile and !$file) {
filename:
	my $description = <<EOF;
What is the name of the file that contains your report?
EOF
	my $entry = _prompt($description, "Filename");

	if ($entry eq "") {
	    paraprint <<EOF;
It seems you didn't enter a filename. Please choose to use a text
editor or enter a filename.
EOF
	    goto editor;
	}

	unless (-f $entry and -r $entry) {
	    paraprint <<EOF;
'$entry' doesn't seem to be a readable file.  You may have mistyped
its name or may not have permission to read it.

If you don't want to use a file as the content of your report, just
hit Enter and you'll be able to select a text editor instead.
EOF
	    goto filename;
	}
	$file = $entry;
    }

    # Generate report
    open(REP, '>:raw', $filename) or die "Unable to create report file '$filename': $!\n";
    binmode(REP, ':raw :crlf') if $Is_MSWin32;

    my $reptype = !$ok ? ($thanks ? 'thank-you' : 'bug')
	: $opt{n} ? "build failure" : "success";

    print REP <<EOF;
This is a $reptype report for perl from $from,
generated with the help of perlbug $VERSION running under perl $perl_version.

EOF

    if ($report_about_module) {
	print REP "Module: $report_about_module\n\n";
    }

    if ($body) {
	print REP $body;
    } elsif ($usefile) {
	open(F, '<:raw', $file)
		or die "Unable to read report file from '$file': $!\n";
	binmode(F, ':raw :crlf') if $Is_MSWin32;
	while (<F>) {
	    print REP $_
	}
	close(F) or die "Error closing '$file': $!";
    } else {
	if ($thanks) {
	    print REP <<'EOF';

-----------------------------------------------------------------
[Please enter your thank-you message here]



[You're welcome to delete anything below this line]
-----------------------------------------------------------------
EOF
	} else {
	    print REP <<'EOF';

-----------------------------------------------------------------
<!--[Please describe your issue here]-->

**Description**
<!-- A clear and concise description of what the bug is. -->



**Steps to Reproduce**
<!-- A one-liner or script to reproduce the issue. -->



**Expected behavior**
<!-- A clear and concise description of what you expected to happen. -->



<!--[Please do not change anything below this line]-->
<!------------------------------------------------------------------- -->
EOF
	}
    }
    Dump(*REP);
    close(REP) or die "Error closing report file: $!";

    # Set up an initial report fingerprint so we can compare it later
    _fingerprint_lines_in_report();

} # sub Query

sub Dump {
    local(*OUT) = @_;

    # these won't have been set if run with -d
    $category ||= 'core';
    $severity ||= 'low';

    print OUT <<EFF;


---
**Flags**
- category=$category
- severity=$severity
EFF

    if ($has_patch) {
        print OUT <<EFF;
- Type=Patch
- PatchStatus=HasPatch
EFF
    }

    if ($report_about_module ) { 
        print OUT <<EFF;
- module=$report_about_module
EFF
    }
    print OUT <<EFF;
---
**Perl configuration**
```
EFF
    print OUT "This perlbug was built using Perl $config_tag1\n",
	    "It is being executed now by  Perl $config_tag2.\n\n"
	if $config_tag2 ne $config_tag1;

    print OUT <<EOF;
Site configuration information for perl $perl_version:

EOF
    if ($::Config{cf_by} and $::Config{cf_time}) {
	print OUT "Configured by $::Config{cf_by} at $::Config{cf_time}.\n\n";
    }
    print OUT Config::myconfig;

    if (@patches) {
	print OUT join "\n    ", "Locally applied patches:", @patches;
	print OUT "\n";
    };

    print OUT <<EOF;

---
\@INC for perl $perl_version:
EOF
    for my $i (@INC) {
	print OUT "    $i\n";
    }

    print OUT <<EOF;

---
Environment for perl $perl_version:
EOF
    my @env =
        qw(PATH LD_LIBRARY_PATH LANG PERL_BADLANG SHELL HOME LOGDIR LANGUAGE);
    push @env, $Config{ldlibpthname} if $Config{ldlibpthname} ne '';
    push @env, grep /^(?:PERL|LC_|LANG|CYGWIN)/, keys %ENV;
    my %env;
    @env{@env} = @env;
    for my $env (sort keys %env) {
	print OUT "    $env",
		exists $ENV{$env} ? "=$ENV{$env}" : ' (unset)',
		"\n";
    }
    if ($verbose) {
	print OUT "\nComplete configuration data for perl $perl_version:\n\n";
	my $value;
	foreach (sort keys %::Config) {
	    $value = $::Config{$_};
	    $value = '' unless defined $value;
	    $value =~ s/'/\\'/g;
	    print OUT "$_='$value'\n";
	}
    }
    print OUT "```\n";
} # sub Dump

sub Edit {
    # Edit the report
    if ($usefile || $body) {
	my $description = "Please make sure that the name of the editor you want to use is correct.";
	my $entry = _prompt($description, 'Editor', $ed);
	$ed = $entry unless $entry eq '';
    }

    _edit_file($ed) unless $running_noninteractively;
}

sub _edit_file {
    my $editor = shift;

    my $report_written = 0;

    while ( !$report_written ) {
        my $exit_status = system("$editor $filename");
        if ($exit_status) {
            my $desc = <<EOF;
The editor you chose ('$editor') could not be run!

If you mistyped its name, please enter it now, otherwise just press Enter.
EOF
            my $entry = _prompt( $desc, 'Editor', $editor );
            if ( $entry ne "" ) {
                $editor = $entry;
                next;
            } else {
                paraprint <<EOF;
You can edit your report after saving it to a file.
EOF
                return;
            }
        }
        return if ( $ok and not $opt{n} ) || $body;

        # Check that we have a report that has some, eh, report in it.

        unless ( _fingerprint_lines_in_report() ) {
            my $description = <<EOF;
It looks like you didn't enter a report. You may [r]etry your edit
or [c]ancel this report.
EOF
            my $action = _prompt( $description, "Action (Retry/Cancel) " );
            if ( $action =~ /^[re]/i ) {    # <R>etry <E>dit
                next;
            } elsif ( $action =~ /^[cq]/i ) {    # <C>ancel, <Q>uit
                Cancel();                        # cancel exits
            }
        }
        # Ok. the user did what they needed to;
        return;

    }
}


sub Cancel {
    1 while unlink($filename);  # remove all versions under VMS
    print "\nQuitting without generating a report.\n";
    exit(0);
}

sub NowWhat {
    # Report is done, prompt for further action
    if( !$opt{S} ) {
	while(1) {
	    my $send_to = $address || 'the Perl developers';
	    my $menu = <<EOF;


You have finished composing your report. At this point, you have 
a few options. You can:

    * Save the report to a [f]ile
    * [Se]nd the report to $send_to$andcc
    * [D]isplay the report on the screen
    * [R]e-edit the report
    * Display or change the report's [su]bject
    * [Q]uit without generating the report

EOF
      retry:
        print $menu;
	    my $action =  _prompt('', "Action (Save/Send/Display/Edit/Subject/Quit)",
	        $opt{t} ? 'q' : '');
        print "\n";
	    if ($action =~ /^(f|sa)/i) { # <F>ile/<Sa>ve
            if ( SaveMessage() ) { exit }
	    } elsif ($action =~ /^(d|l|sh)/i ) { # <D>isplay, <L>ist, <Sh>ow
		# Display the message
		print _read_report($filename);
		if ($have_attachment) {
		    print "\n\n---\nAttachment(s):\n";
		    for my $att (split /\s*,\s*/, $attachments) { print "    $att\n"; }
		}
	    } elsif ($action =~ /^su/i) { # <Su>bject
		my $reply = _prompt( "Subject: $subject", "If the above subject is fine, press Enter. Otherwise, type a replacement now\nSubject");
		if ($reply ne '') {
		    unless (TrivialSubject($reply)) {
			$subject = $reply;
			print "Subject: $subject\n";
		    }
		}
	    } elsif ($action =~ /^se/i) { # <S>end
		# Send the message
		if (not $thanks) {
		    print <<EOF
To ensure your issue can be best tracked and resolved,
you should submit it to the GitHub issue tracker at
https://github.com/Perl/perl5/issues
EOF
		}
		my $reply =  _prompt( "Are you certain you want to send this report to $send_to$andcc?", 'Please type "yes" if you are','no');
		if ($reply =~ /^yes$/) {
		    $address ||= 'perl5-porters@perl.org';
		    last;
		} else {
		    paraprint <<EOF;
You didn't type "yes", so your report has not been sent.
EOF
		}
	    } elsif ($action =~ /^[er]/i) { # <E>dit, <R>e-edit
		# edit the message
		Edit();
	    } elsif ($action =~ /^[qc]/i) { # <C>ancel, <Q>uit
		Cancel();
	    } elsif ($action =~ /^s/i) {
		paraprint <<EOF;
The command you entered was ambiguous. Please type "send", "save" or "subject".
EOF
	    }
	}
    }
} # sub NowWhat

sub TrivialSubject {
    my $subject = shift;
    if ($subject =~
	/^(y(es)?|no?|help|perl( (bug|problem))?|bug|problem)$/i ||
	length($subject) < 4 ||
	($subject !~ /\s/ && ! $opt{t})) { # non-whitespace is accepted in test mode
	print "\nThe subject you entered wasn't very descriptive. Please try again.\n\n";
        return 1;
    } else {
	return 0;
    }
}

sub SaveMessage {
    my $file = _prompt( '', "Name of file to save report in", $outfile );
    save_message_to_disk($file) || return undef;
    return 1;
}

sub Send {

    # Message has been accepted for transmission -- Send the message

    # on linux certain "mail" implementations won't accept the subject
    # as "~s subject" and thus the Subject header will be corrupted
    # so don't use Mail::Send to be safe
    eval {
        if ( $::HaveSend && !$Is_Linux && !$Is_OpenBSD ) {
            _send_message_mailsend();
        } elsif ($Is_VMS) {
            _send_message_vms();
        } else {
            _send_message_sendmail();
        }
    };

    if ( my $error = $@ ) {
        paraprint <<EOF;
$0 has detected an error while trying to send your message: $error.

Your message may not have been sent. You will now have a chance to save a copy to disk.
EOF
        SaveMessage();
        return;
    }

    1 while unlink($filename);    # remove all versions under VMS
}    # sub Send

sub Help {
    print <<EOF;

This program is designed to help you generate bug reports
(and thank-you notes) about perl5 and the modules which ship with it.

In most cases, you can just run "$0" interactively from a command
line without any special arguments and follow the prompts.

Advanced usage:

$0  [-v] [-a address] [-s subject] [-b body | -f inpufile ] [ -F outputfile ]
    [-r returnaddress] [-e editor] [-c adminaddress | -C] [-S] [-t] [-h]
    [-p patchfile ]
$0  [-v] [-r returnaddress] [-ok | -okay | -nok | -nokay]


Options:

  -v    Include Verbose configuration data in the report
  -f    File containing the body of the report. Use this to
        quickly send a prepared report.
  -p    File containing a patch or other text attachment. Separate
        multiple files with commas.
  -F    File to output the resulting report to. Defaults to
        '$outfile'.
  -S    Save or send the report without asking for confirmation.
  -a    Send the report to this address, instead of saving to a file.
  -c    Address to send copy of report to. Defaults to '$cc'.
  -C    Don't send copy to administrator.
  -s    Subject to include with the report. You will be prompted
        if you don't supply one on the command line.
  -b    Body of the report. If not included on the command line, or
        in a file with -f, you will get a chance to edit the report.
  -r    Your return address. The program will ask you to confirm
        this if you don't give it here.
  -e    Editor to use.
  -t    Test mode.
  -T    Thank-you mode. The target address defaults to '$thanksaddress'.
  -d    Data mode.  This prints out your configuration data, without mailing
        anything. You can use this with -v to get more complete data.
  -ok   Report successful build on this system to perl porters
        (use alone or with -v). Only use -ok if *everything* was ok:
        if there were *any* problems at all, use -nok.
  -okay As -ok but allow report from old builds.
  -nok  Report unsuccessful build on this system to perl porters
        (use alone or with -v). You must describe what went wrong
        in the body of the report which you will be asked to edit.
  -nokay As -nok but allow report from old builds.
  -h    Print this help message.

EOF
}

sub filename {
    if ($::HaveTemp) {
	# Good. Use a secure temp file
	my ($fh, $filename) = File::Temp::tempfile(UNLINK => 1);
	close($fh);
	return $filename;
    } else {
	# Bah. Fall back to doing things less securely.
	my $dir = File::Spec->tmpdir();
	$filename = "bugrep0$$";
	$filename++ while -e File::Spec->catfile($dir, $filename);
	$filename = File::Spec->catfile($dir, $filename);
    }
}

sub paraprint {
    my @paragraphs = split /\n{2,}/, "@_";
    for (@paragraphs) {   # implicit local $_
	s/(\S)\s*\n/$1 /g;
	write;
	print "\n";
    }
}

sub _prompt {
    my ($explanation, $prompt, $default) = (@_);
    if ($explanation) {
        print "\n\n";
        paraprint $explanation;
    }
    print $prompt. ($default ? " [$default]" :''). ": ";
	my $result = scalar(<>);
    return $default if !defined $result; # got eof
    chomp($result);
	$result =~ s/^\s*(.*?)\s*$/$1/s;
    if ($default && $result eq '') {
        return $default;
    } else {
        return $result;
    }
}

sub _build_header {
    my %attr = (@_);

    my $head = '';
    for my $header (keys %attr) {
        $head .= "$header: ".$attr{$header}."\n";
    }
    return $head;
}

sub _message_headers {
    my %headers = ( To => $address || 'perl5-porters@perl.org', Subject => $subject );
    $headers{'Cc'}         = $cc        if ($cc);
    $headers{'Message-Id'} = $messageid if ($messageid);
    $headers{'Reply-To'}   = $from      if ($from);
    $headers{'From'}       = $from      if ($from);
    if ($have_attachment) {
        $headers{'MIME-Version'} = '1.0';
        $headers{'Content-Type'} = qq{multipart/mixed; boundary=\"$mime_boundary\"};
    }
    return \%headers;
}

sub _add_body_start {
    my $body_start = <<"BODY_START";
This is a multi-part message in MIME format.
--$mime_boundary
Content-Type: text/plain; format=fixed
Content-Transfer-Encoding: 8bit

BODY_START
    return $body_start;
}

sub _add_attachments {
    my $attach = '';
    for my $attachment (split /\s*,\s*/, $attachments) {
        my $attach_file = basename($attachment);
        $attach .= <<"ATTACHMENT";

--$mime_boundary
Content-Type: text/x-patch; name="$attach_file"
Content-Transfer-Encoding: 8bit
Content-Disposition: attachment; filename="$attach_file"

ATTACHMENT

        open my $attach_fh, '<:raw', $attachment
            or die "Couldn't open attachment '$attachment': $!\n";
        while (<$attach_fh>) { $attach .= $_; }
        close($attach_fh) or die "Error closing attachment '$attachment': $!";
    }

    $attach .= "\n--$mime_boundary--\n";
    return $attach;
}

sub _read_report {
    my $fname = shift;
    my $content;
    open( REP, "<:raw", $fname ) or die "Couldn't open file '$fname': $!\n";
    binmode(REP, ':raw :crlf') if $Is_MSWin32;
    # wrap long lines to make sure the report gets delivered
    local $Text::Wrap::columns = 900;
    local $Text::Wrap::huge = 'overflow';
    while (<REP>) {
        if ($::HaveWrap && /\S/) { # wrap() would remove empty lines
            $content .= Text::Wrap::wrap(undef, undef, $_);
        } else {
            $content .= $_;
        }
    }
    close(REP) or die "Error closing report file '$fname': $!";
    return $content;
}

sub build_complete_message {
    my $content = _build_header(%{_message_headers()}) . "\n\n";
    $content .= _add_body_start() if $have_attachment;
    $content .= _read_report($filename);
    $content .= _add_attachments() if $have_attachment;
    return $content;
}

sub save_message_to_disk {
    my $file = shift;

        if (-e $file) {
            my $response = _prompt( '', "Overwrite existing '$file'", 'n' );
            return undef unless $response =~ / yes | y /xi;
        }
        open OUTFILE, '>:raw', $file or do { warn  "Couldn't open '$file': $!\n"; return undef};
        binmode(OUTFILE, ':raw :crlf') if $Is_MSWin32;

        print OUTFILE build_complete_message();
        close(OUTFILE) or do { warn  "Error closing $file: $!"; return undef };
	    print "\nReport saved to '$file'. Please submit it to https://github.com/Perl/perl5/issues\n";
        return 1;
}

sub _send_message_vms {

    my $mail_from  = $from;
    my $rcpt_to_to = $address;
    my $rcpt_to_cc = $cc;

    map { $_ =~ s/^[^<]*<//;
          $_ =~ s/>[^>]*//; } ($mail_from, $rcpt_to_to, $rcpt_to_cc);

    if ( open my $sff_fh, '|-:raw', 'MCR TCPIP$SYSTEM:TCPIP$SMTP_SFF.EXE SYS$INPUT:' ) {
        print $sff_fh "MAIL FROM:<$mail_from>\n";
        print $sff_fh "RCPT TO:<$rcpt_to_to>\n";
        print $sff_fh "RCPT TO:<$rcpt_to_cc>\n" if $rcpt_to_cc;
        print $sff_fh "DATA\n";
        print $sff_fh build_complete_message();
        my $success = close $sff_fh;
        if ($success ) {
            print "\nMessage sent\n";
            return;
        }
    }
    die "Mail transport failed (leaving bug report in $filename): $^E\n";
}

sub _send_message_mailsend {
    my $msg = Mail::Send->new();
    my %headers = %{_message_headers()};
    for my $key ( keys %headers) {
        $msg->add($key => $headers{$key});
    }

    $fh = $msg->open;
    binmode($fh, ':raw');
    print $fh _add_body_start() if $have_attachment;
    print $fh _read_report($filename);
    print $fh _add_attachments() if $have_attachment;
    $fh->close or die "Error sending mail: $!";

    print "\nMessage sent.\n";
}

sub _probe_for_sendmail {
    my $sendmail = "";
    for (qw(/usr/lib/sendmail /usr/sbin/sendmail /usr/ucblib/sendmail)) {
        $sendmail = $_, last if -e $_;
    }
    if ( $^O eq 'os2' and $sendmail eq "" ) {
        my $path = $ENV{PATH};
        $path =~ s:\\:/:;
        my @path = split /$Config{'path_sep'}/, $path;
        for (@path) {
            $sendmail = "$_/sendmail",     last if -e "$_/sendmail";
            $sendmail = "$_/sendmail.exe", last if -e "$_/sendmail.exe";
        }
    }
    return $sendmail;
}

sub _send_message_sendmail {
    my $sendmail = _probe_for_sendmail();
    unless ($sendmail) {
        my $message_start = !$Is_Linux && !$Is_OpenBSD ? <<'EOT' : <<'EOT';
It appears that there is no program which looks like "sendmail" on
your system and that the Mail::Send library from CPAN isn't available.
EOT
It appears that there is no program which looks like "sendmail" on
your system.
EOT
        paraprint(<<"EOF"), die "\n";
$message_start
Because of this, there's no easy way to automatically send your
report.

A copy of your report has been saved in '$filename' for you to
send to '$address' with your normal mail client.
EOF
    }

    open( SENDMAIL, "|-:raw", $sendmail, "-t", "-oi", "-f", $from )
        || die "'|$sendmail -t -oi -f $from' failed: $!";
    print SENDMAIL build_complete_message();
    if ( close(SENDMAIL) ) {
        print "\nMessage sent\n";
    } else {
        warn "\nSendmail returned status '", $? >> 8, "'\n";
    }
}



# a strange way to check whether any significant editing
# has been done: check whether any new non-empty lines
# have been added.

sub _fingerprint_lines_in_report {
    my $new_lines = 0;
    # read in the report template once so that
    # we can track whether the user does any editing.
    # yes, *all* whitespace is ignored.

    open(REP, '<:raw', $filename) or die "Unable to open report file '$filename': $!\n";
    binmode(REP, ':raw :crlf') if $Is_MSWin32;
    while (my $line = <REP>) {
        $line =~ s/\s+//g;
        $new_lines++ if (!$REP{$line});

    }
    close(REP) or die "Error closing report file '$filename': $!";
    # returns the number of lines with content that wasn't there when last we looked
    return $new_lines;
}



format STDOUT =
^<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ~~
$_
.

__END__

=head1 NAME

perlbug - how to submit bug reports on Perl

=head1 SYNOPSIS

B<perlbug>

B<perlbug> S<[ B<-v> ]> S<[ B<-a> I<address> ]> S<[ B<-s> I<subject> ]>
S<[ B<-b> I<body> | B<-f> I<inputfile> ]> S<[ B<-F> I<outputfile> ]>
S<[ B<-r> I<returnaddress> ]>
S<[ B<-e> I<editor> ]> S<[ B<-c> I<adminaddress> | B<-C> ]>
S<[ B<-S> ]> S<[ B<-t> ]>  S<[ B<-d> ]>  S<[ B<-h> ]> S<[ B<-T> ]>

B<perlbug> S<[ B<-v> ]> S<[ B<-r> I<returnaddress> ]>
 S<[ B<-ok> | B<-okay> | B<-nok> | B<-nokay> ]>

B<perlthanks>

=head1 DESCRIPTION


This program is designed to help you generate bug reports
(and thank-you notes) about perl5 and the modules which ship with it.

In most cases, you can just run it interactively from a command
line without any special arguments and follow the prompts.

If you have found a bug with a non-standard port (one that was not
part of the I<standard distribution>), a binary distribution, or a
non-core module (such as Tk, DBI, etc), then please see the
documentation that came with that distribution to determine the
correct place to report bugs.

Bug reports should be submitted to the GitHub issue tracker at
L<https://github.com/Perl/perl5/issues>. The B<perlbug@perl.org>
address no longer automatically opens tickets. You can use this tool
to compose your report and save it to a file which you can then submit
to the issue tracker.

In extreme cases, B<perlbug> may not work well enough on your system
to guide you through composing a bug report. In those cases, you
may be able to use B<perlbug -d> or B<perl -V> to get system
configuration information to include in your issue report.


When reporting a bug, please run through this checklist:

=over 4

=item What version of Perl you are running?

Type C<perl -v> at the command line to find out.

=item Are you running the latest released version of perl?

Look at L<http://www.perl.org/> to find out.  If you are not using the
latest released version, please try to replicate your bug on the
latest stable release.

Note that reports about bugs in old versions of Perl, especially
those which indicate you haven't also tested the current stable
release of Perl, are likely to receive less attention from the
volunteers who build and maintain Perl than reports about bugs in
the current release.

=item Are you sure what you have is a bug?

A significant number of the bug reports we get turn out to be
documented features in Perl.  Make sure the issue you've run into
isn't intentional by glancing through the documentation that comes
with the Perl distribution.

Given the sheer volume of Perl documentation, this isn't a trivial
undertaking, but if you can point to documentation that suggests
the behaviour you're seeing is I<wrong>, your issue is likely to
receive more attention. You may want to start with B<perldoc>
L<perltrap> for pointers to common traps that new (and experienced)
Perl programmers run into.

If you're unsure of the meaning of an error message you've run
across, B<perldoc> L<perldiag> for an explanation.  If the message
isn't in perldiag, it probably isn't generated by Perl.  You may
have luck consulting your operating system documentation instead.

If you are on a non-UNIX platform B<perldoc> L<perlport>, as some
features may be unimplemented or work differently.

You may be able to figure out what's going wrong using the Perl
debugger.  For information about how to use the debugger B<perldoc>
L<perldebug>.

=item Do you have a proper test case?

The easier it is to reproduce your bug, the more likely it will be
fixed -- if nobody can duplicate your problem, it probably won't be 
addressed.

A good test case has most of these attributes: short, simple code;
few dependencies on external commands, modules, or libraries; no
platform-dependent code (unless it's a platform-specific bug);
clear, simple documentation.

A good test case is almost always a good candidate to be included in
Perl's test suite.  If you have the time, consider writing your test case so
that it can be easily included into the standard test suite.

=item Have you included all relevant information?

Be sure to include the B<exact> error messages, if any.
"Perl gave an error" is not an exact error message.

If you get a core dump (or equivalent), you may use a debugger
(B<dbx>, B<gdb>, etc) to produce a stack trace to include in the bug
report.

NOTE: unless your Perl has been compiled with debug info
(often B<-g>), the stack trace is likely to be somewhat hard to use
because it will most probably contain only the function names and not
their arguments.  If possible, recompile your Perl with debug info and
reproduce the crash and the stack trace.

=item Can you describe the bug in plain English?

The easier it is to understand a reproducible bug, the more likely
it will be fixed.  Any insight you can provide into the problem
will help a great deal.  In other words, try to analyze the problem
(to the extent you can) and report your discoveries.

=item Can you fix the bug yourself?

If so, that's great news; bug reports with patches are likely to
receive significantly more attention and interest than those without
patches.  Please submit your patch via the GitHub Pull Request workflow
as described in B<perldoc> L<perlhack>.  You may also send patches to
B<perl5-porters@perl.org>.  When sending a patch, create it using
C<git format-patch> if possible, though a unified diff created with
C<diff -pu> will do nearly as well.

Your patch may be returned with requests for changes, or requests for more
detailed explanations about your fix.

Here are a few hints for creating high-quality patches:

Make sure the patch is not reversed (the first argument to diff is
typically the original file, the second argument your changed file).
Make sure you test your patch by applying it with C<git am> or the
C<patch> program before you send it on its way.  Try to follow the
same style as the code you are trying to patch.  Make sure your patch
really does work (C<make test>, if the thing you're patching is covered
by Perl's test suite).

=item Can you use C<perlbug> to submit a thank-you note?

Yes, you can do this by either using the C<-T> option, or by invoking
the program as C<perlthanks>. Thank-you notes are good. It makes people
smile. 

=back

Please make your issue title informative.  "a bug" is not informative.
Neither is "perl crashes" nor is "HELP!!!".  These don't help.  A compact
description of what's wrong is fine.

Having done your bit, please be prepared to wait, to be told the
bug is in your code, or possibly to get no reply at all.  The
volunteers who maintain Perl are busy folks, so if your problem is
an obvious bug in your own code, is difficult to understand or is
a duplicate of an existing report, you may not receive a personal
reply.

If it is important to you that your bug be fixed, do monitor the
issue tracker (you will be subscribed to notifications for issues you
submit or comment on) and the commit logs to development
versions of Perl, and encourage the maintainers with kind words or
offers of frosty beverages.  (Please do be kind to the maintainers.
Harassing or flaming them is likely to have the opposite effect of the
one you want.)

Feel free to update the ticket about your bug on
L<https://github.com/Perl/perl5/issues>
if a new version of Perl is released and your bug is still present.

=head1 OPTIONS

=over 8

=item B<-a>

Address to send the report to instead of saving to a file.

=item B<-b>

Body of the report.  If not included on the command line, or
in a file with B<-f>, you will get a chance to edit the report.

=item B<-C>

Don't send copy to administrator when sending report by mail.

=item B<-c>

Address to send copy of report to when sending report by mail.
Defaults to the address of the
local perl administrator (recorded when perl was built).

=item B<-d>

Data mode (the default if you redirect or pipe output).  This prints out
your configuration data, without saving or mailing anything.  You can use
this with B<-v> to get more complete data.

=item B<-e>

Editor to use.

=item B<-f>

File containing the body of the report.  Use this to quickly send a
prepared report.

=item B<-F>

File to output the results to.  Defaults to B<perlbug.rep>.

=item B<-h>

Prints a brief summary of the options.

=item B<-ok>

Report successful build on this system to perl porters. Forces B<-S>
and B<-C>. Forces and supplies values for B<-s> and B<-b>. Only
prompts for a return address if it cannot guess it (for use with
B<make>). Honors return address specified with B<-r>.  You can use this
with B<-v> to get more complete data.   Only makes a report if this
system is less than 60 days old.

=item B<-okay>

As B<-ok> except it will report on older systems.

=item B<-nok>

Report unsuccessful build on this system.  Forces B<-C>.  Forces and
supplies a value for B<-s>, then requires you to edit the report
and say what went wrong.  Alternatively, a prepared report may be
supplied using B<-f>.  Only prompts for a return address if it
cannot guess it (for use with B<make>). Honors return address
specified with B<-r>.  You can use this with B<-v> to get more
complete data.  Only makes a report if this system is less than 60
days old.

=item B<-nokay>

As B<-nok> except it will report on older systems.

=item B<-p>

The names of one or more patch files or other text attachments to be
included with the report.  Multiple files must be separated with commas.

=item B<-r>

Your return address.  The program will ask you to confirm its default
if you don't use this option.

=item B<-S>

Save or send the report without asking for confirmation.

=item B<-s>

Subject to include with the report.  You will be prompted if you don't
supply one on the command line.

=item B<-t>

Test mode.  Makes it possible to command perlbug from a pipe or file, for
testing purposes.

=item B<-T>

Send a thank-you note instead of a bug report. 

=item B<-v>

Include verbose configuration data in the report.

=back

=head1 AUTHORS

Kenneth Albanowski (E<lt>kjahds@kjahds.comE<gt>), subsequently
I<doc>tored by Gurusamy Sarathy (E<lt>gsar@activestate.comE<gt>),
Tom Christiansen (E<lt>tchrist@perl.comE<gt>), Nathan Torkington
(E<lt>gnat@frii.comE<gt>), Charles F. Randall (E<lt>cfr@pobox.comE<gt>),
Mike Guy (E<lt>mjtg@cam.ac.ukE<gt>), Dominic Dunlop
(E<lt>domo@computer.orgE<gt>), Hugo van der Sanden (E<lt>hv@crypt.orgE<gt>),
Jarkko Hietaniemi (E<lt>jhi@iki.fiE<gt>), Chris Nandor
(E<lt>pudge@pobox.comE<gt>), Jon Orwant (E<lt>orwant@media.mit.eduE<gt>,
Richard Foley (E<lt>richard.foley@rfi.netE<gt>), Jesse Vincent
(E<lt>jesse@bestpractical.comE<gt>), and Craig A. Berry (E<lt>craigberry@mac.comE<gt>).

=head1 SEE ALSO

perl(1), perldebug(1), perldiag(1), perlport(1), perltrap(1),
diff(1), patch(1), dbx(1), gdb(1)

=head1 BUGS

None known (guess what must have been used to report them?)

=cut

!NO!SUBS!

close OUT or die "Can't close $file: $!";
chmod 0755, $file or die "Can't reset permissions for $file: $!\n";
exec("$Config{'eunicefix'} $file") if $Config{'eunicefix'} ne ':';
chdir $origdir;
