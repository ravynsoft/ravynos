#!./perl

# The tests are in a separate file 't/re/re_tests'.
# Each line in that file is a separate test.
# There are five columns, separated by tabs.
# An optional sixth column is used to give a reason, only when skipping tests
#
# Column 1 contains the pattern, optionally enclosed in C<''> C<::> or
# C<//>.  Modifiers can be put after the closing delimiter.  C<''> will
# automatically be added to any other patterns.
#
# Column 2 contains the string to be matched.
#
# Column 3 contains the expected result:
# 	y	expect a match
# 	n	expect no match
# 	c	expect an error
#	T	the test is a TODO (can be combined with y/n/c)
#	M	skip test on miniperl (combine with y/n/c/T)
#	B	test exposes a known bug in Perl, should be skipped
#	b	test exposes a known bug in Perl, should be skipped if noamp
#	t	test exposes a bug with threading, TODO if qr_embed_thr
#       s       test should only be run for regex_sets_compat.t
#       S       test should not be run for regex_sets_compat.t
#       a       test should only be run on ASCII platforms
#       e       test should only be run on EBCDIC platforms
#
# Columns 4 and 5 are used only if column 3 contains C<y> or C<c>.
#
# Column 4 contains a string, usually C<$&>.
#
# Column 5 contains the expected result of double-quote
# interpolating that string after the match, or start of error message.
#
# Column 6, if present, contains a reason why the test is skipped.
# This is printed with "skipped", for harness to pick up.
#
# Column 7 can be used for comments
#
# \n in the tests are interpolated, as are variables of the form ${\w+}.
#
# Blanks lines are treated as PASSING tests to keep the line numbers
# linked to the test number.
#
# If you want to add a regular expression test that can't be expressed
# in this format, don't add it here: put it in re/pat.t instead.
#
# Note that the inputs get passed on as "m're'", so the re bypasses the lexer.
# This means this file cannot be used for testing anything that the lexer
# handles; in 5.12 this means just \N{NAME} and \N{U+...}.
#
# Note that columns 2,3 and 5 are all enclosed in double quotes and then
# evalled; so something like a\"\x{100}$1 has length 3+length($1).
#
# \x... and \o{...} constants are automatically converted to the native
# character set if necessary.  \[0-7] constants aren't

my ($file, $iters);
BEGIN {
    $iters = shift || 1;	# Poor man performance suite, 10000 is OK.

    # Do this open before any chdir
    $file = shift;
    if (defined $file) {
	open TESTS, $file or die "Can't open $file";
    }

    chdir 't' if -d 't';
    @INC = qw '../lib ../ext/re';
    if (!defined &DynaLoader::boot_DynaLoader) { # miniperl
	print("1..0 # Skip Unicode tables not built yet\n"), exit
	    unless eval 'require "unicore/UCD.pl"';
    }

    # Some of the tests need a locale; which one doesn't much matter, except
    # that it be valid.  Make sure of that
    eval { require POSIX;
            POSIX->import(qw(LC_ALL setlocale));
            POSIX::setlocale(&LC_ALL, "C");
    };
}

sub _comment {
    return map { /^#/ ? "$_\n" : "# $_\n" }
           map { split /\n/ } @_;
}

use strict;
use warnings FATAL=>"all";
no warnings 'experimental::vlb';
our ($bang, $ffff, $nulnul); # used by the tests
our ($qr, $skip_amp, $qr_embed, $qr_embed_thr, $regex_sets, $alpha_assertions, $no_null); # set by our callers

if ($no_null && ! eval { require XS::APItest }) {
    print("1..0 # Skip XS::APItest not available\n"), exit
}

my $expanded_text = "expanded name from original test number";
my $expanded_text_re = qr/$expanded_text/;

if (!defined $file) {
    open TESTS, 're/re_tests' or die "Can't open re/re_tests: $!";
}

my @tests = <TESTS>;

close TESTS;

my $test_num = 0;

# Some scenarios add extra tests to those just read in.  For those where there
# is a character set translation, the added test will already have been
# translated, so any test number beginning with this one shouldn't be
# translated again.
my $first_already_converted_test_num = @tests + 1;

sub convert_from_ascii_guts {
    my $string_ref = shift;

    return if $test_num >= $first_already_converted_test_num;

    #my $save = $string_ref;
    # Convert \x{...}, \o{...}
    $$string_ref =~ s/ (?<! \\ ) \\x\{ ( .*? ) } / "\\x{" . sprintf("%X", utf8::unicode_to_native(hex $1)) .  "}" /gex;
    $$string_ref =~ s/ (?<! \\ ) \\o\{ ( .*? ) } / "\\o{" . sprintf("%o", utf8::unicode_to_native(oct $1)) .  "}" /gex;

    # Convert \xAB
    $$string_ref =~ s/ (?<! \\ ) \\x ( [A-Fa-f0-9]{2} ) / "\\x" . sprintf("%02X", utf8::unicode_to_native(hex $1)) /gex;

    # Convert \xA
    $$string_ref =~ s/ (?<! \\ ) \\x ( [A-Fa-f0-9] ) (?! [A-Fa-f0-9] ) / "\\x" . sprintf("%X", utf8::unicode_to_native(hex $1)) /gex;

    #print STDERR __LINE__, ": $save\n$string_ref\n" if $save ne $string_ref;
    return;
}

*convert_from_ascii = (ord("A") == 65)
                      ? sub { 1; }
                      : \&convert_from_ascii_guts;

$bang = sprintf "\\%03o", ord "!"; # \41 would not be portable.
$ffff  = chr(0xff) x 2;
$nulnul = "\0" x 2;
my $OP = $qr ? 'qr' : 'm';

$| = 1;
$::normalize_pat = $::normalize_pat; # silence warning
TEST:
foreach (@tests) {
    $test_num++;
    if (!/\S/ || /^\s*#/ || /^__END__$/) {
        chomp;
        my ($not,$comment)= split /\s*#\s*/, $_, 2;
        $comment ||= "(blank line)";
        print "ok $test_num # $comment\n";
        next;
    }
    chomp;
    s/\\n/\n/g unless $regex_sets;
    my ($pat, $subject, $result, $repl, $expect, $reason, $comment) = split(/\t/,$_,7);
    $comment = "" unless defined $comment;
    if (!defined $subject) {
        die "Bad test definition on line $test_num: $_\n";
    }
    $reason = '' unless defined $reason;
    my $input = join(':',$pat,$subject,$result,$repl,$expect);

    # the double '' below keeps simple syntax highlighters from going crazy
    $pat = "'$pat'" unless $pat =~ /^[:''\/]/;
    $pat =~ s/(\$\{\w+\})/$1/eeg;
    $pat =~ s/\\n/\n/g unless $regex_sets;
    convert_from_ascii(\$pat);

    my $no_null_pat;
    if ($no_null && $pat =~ /^'(.*)'\z/) {
       $no_null_pat = XS::APItest::string_without_null($1);
    }

    convert_from_ascii(\$subject);
    $subject = eval qq("$subject"); die $@ if $@;

    convert_from_ascii(\$expect);
    $expect  = eval qq("$expect"); die $@ if $@;
    my $has_amp = $input =~ /\$[&\`\']/;
    $expect = $repl = '-' if $skip_amp and $has_amp;

    my $todo_qr = $qr_embed_thr && ($result =~ s/t//);
    my $skip = ($skip_amp ? ($result =~ s/B//i) : ($result =~ s/B//));
    ++$skip if $result =~ s/M// && !defined &DynaLoader::boot_DynaLoader;
    
    if ($::normalize_pat) {
        my $opat= $pat;
        # Convert (x)? to (?:(x)|) and (x)+ to (?:(x))+ and (x)* to (?:(x))*
        $pat =~ s/\(([\w|.]+)\)\?(?![+*?])/(?:($1)|)/g;
        $pat =~ s/\(([\w|.]+)\)([+*])(?![+*?])/(?:($1))$2/g;
        if ($opat eq $pat) {
            # we didn't change anything, no point in testing it again.
            $skip++;
            $reason = "Test not valid for $0";
        } elsif ($comment=~/!\s*normal/) {
            $result .= "T";
            $comment = "# Known to be broken under $0";
        }
    }

    if ($result =~ s/ ( [Ss] ) //x) {
        if (($1 eq 'S' && $regex_sets) || ($1 eq 's' && ! $regex_sets)) {
            $skip++;
            $reason = "Test not valid for $0";
        }
    }
    if ($result =~ s/a// && ord("A") != 65) {
        $skip++;
        $reason = "Test is only valid for ASCII platforms.  $reason";
    }
    if ($result =~ s/e// && ord("A") != 193) {
        $skip++;
        $reason = "Test is only valid for EBCDIC platforms.  $reason";
    }
    $reason = 'skipping $&' if $reason eq  '' && $skip_amp;
    $result =~ s/B//i unless $skip;
    my $todo= ($result =~ s/T// && (!$skip_amp || !$has_amp)) ? " # TODO" : "";
    my $testname= $test_num;
    if ($comment) {
        $comment=~s/^\s*(?:#\s*)?//;
        $testname .= " - $comment" if $comment;
    }
    if (! $skip && $alpha_assertions) {
        my $assertions_re = qr/ (?: \Q(?\E (?: > | <? [=>] ) ) /x;
        if ($pat !~ $assertions_re && $comment !~ $expanded_text_re) {
            $skip++;
            $reason = "Pattern doesn't contain assertions";
        }
        elsif ($comment !~ $expanded_text_re) {
            my $expanded_pat = $pat;

            $pat =~ s/\( \? > /(*atomic:/xg;

            if ($pat =~ s/\( \? = /(*pla:/xg) {
                $expanded_pat =~ s//(*positive_lookahead:/g;
            }
            if ($pat =~ s/\( \? ! /(*nla:/xg) {
                $expanded_pat =~ s//(*negative_lookahead:/g;
            }
            if ($pat =~ s/\( \? <= /(*plb:/xg) {
                $expanded_pat =~ s//(*positive_lookbehind:/g;
            }
            if ($pat =~ s/\( \? <! /(*nlb:/xg) {
                $expanded_pat =~ s//(*negative_lookbehind:/g;
            }
            if ($expanded_pat ne $pat) {
                $comment .= " $expanded_text $test_num";
                push @tests, join "\t", $expanded_pat,
                                        $subject // "",
                                        $result // "",
                                        $repl // "",
                                        $expect // "",
                                        $reason // "",
                                        $comment;
            }
        }
    }
    elsif (! $skip && $regex_sets) {

        # If testing regex sets, change the [bracketed] classes into
        # (?[bracketed]).  But note that '\[' and '\c[' don't introduce such a
        # class.  (We don't bother looking for an odd number of backslashes,
        # as this hasn't been needed so far.)
        if ($pat !~ / (?<!\\c) (?<!\\) \[ /x) {
            $skip++;
            $reason = "Pattern doesn't contain [brackets]";
        }
        else { # Use non-regex features of Perl to accomplish this.
            my $modified = "";
            my $in_brackets = 0;

            # Go through the pattern character-by-character.  We also add
            # blanks around each token to test the /x parts of (?[ ])
            my $pat_len = length($pat);
      CHAR: for (my $i = 0; $i < $pat_len; $i++) {
                my $curchar = substr($pat, $i, 1);
                if ($curchar eq '\\') {
                    $modified .= " " if $in_brackets;
                    $modified .= $curchar;
                    $i++;

                    # Get the character the backslash is escaping
                    $curchar = substr($pat, $i, 1);
                    $modified .= $curchar;

                    # If the character following that is a '{}', treat the
                    # entire amount as a single token
                    if ($i < $pat_len -1 && substr($pat, $i+1, 1) eq '{') {
                        my $j = index($pat, '}', $i+2);
                        if ($j < 0) {
                            last unless $in_brackets;
                            if ($result eq 'c') {
                                $skip++;
                                $reason = "Can't handle compilation errors with unmatched '{'";
                            }
                            else {
                                print "not ok $testname # Problem in $0; original = '$pat'; mod = '$modified'\n";
                                next TEST;
                            }
                        }
                        $modified .= substr($pat, $i+1, $j - $i);
                        $i = $j;
                    }
                    elsif ($curchar eq 'x') {

                        # \x without brackets is supposed to be followed by 2
                        # hex digits.  Take up to 2, and then add a blank
                        # after the last one.  This avoids getting errors from
                        # (?[ ]) for run-ons, like \xabc
                        my $j = $i + 1;
                        for (; $j < $i + 3 && $j < $pat_len; $j++) {
                            my $curord = ord(substr($pat, $j, 1));
                            if (!(($curord >= ord("A") && $curord <= ord("F"))
                                 || ($curord >= ord("a") && $curord <= ord("f"))
                                 || ($curord >= ord("0") && $curord <= ord("9"))))
                            {
                                $j++;
                                last;
                            }
                        }
                        $j--;
                        $modified .= substr($pat, $i + 1, $j - $i);
                        $modified .= " " if $in_brackets;
                        $i = $j;
                    }
                    elsif (ord($curchar) >= ord('0')
                           && (ord($curchar) <= ord('7')))
                    {
                        # Similarly, octal constants have up to 3 digits.
                        my $j = $i + 1;
                        for (; $j < $i + 3 && $j < $pat_len; $j++) {
                            my $curord = ord(substr($pat, $j, 1));
                            if (! ($curord >= ord("0") &&  $curord <= ord("7"))) {
                                $j++;
                                last;
                            }
                        }
                        $j--;
                        $modified .= substr($pat, $i + 1, $j - $i);
                        $i = $j;
                    }

                    next;
                } # End of processing a backslash sequence

                if (! $in_brackets  # Skip (?{ })
                    && $curchar eq '('
                    && $i < $pat_len - 2
                    && substr($pat, $i+1, 1) eq '?'
                    && substr($pat, $i+2, 1) eq '{')
                {
                    $skip++;
                    $reason = "Pattern contains '(?{'";
                    last;
                }

                # Closing ']'
                if ($curchar eq ']' && $in_brackets) {
                    $modified .= " ] ])";
                    $in_brackets = 0;
                    next;
                }

                # A regular character.
                if ($curchar ne '[') {
                    $modified .= " " if  $in_brackets;
                    $modified .= $curchar;
                    next;
                }

                # Here is a '['; If not in a bracketed class, treat as the
                # beginning of one.
                if (! $in_brackets) {
                    $in_brackets = 1;
                    $modified .= "(?[ [ ";

                    # An immediately following ']' or '^]' is not the ending
                    # of the class, but is to be treated literally.
                    if ($i < $pat_len - 1
                        && substr($pat, $i+1, 1) eq ']')
                    {
                        $i ++;
                        $modified .= " ] ";
                    }
                    elsif ($i < $pat_len - 2
                            && substr($pat, $i+1, 1) eq '^'
                            && substr($pat, $i+2, 1) eq ']')
                    {
                        $i += 2;
                        $modified .= " ^ ] ";
                    }
                    next;
                }

                # Here is a plain '[' within [ ].  Could mean wants to
                # match a '[', or it could be a posix class that has a
                # corresponding ']'.  Absorb either

                $modified .= ' [';
                last if $i >= $pat_len - 1;

                $i++;
                $curchar = substr($pat, $i, 1);
                if ($curchar =~ /[:=.]/) {
                    for (my $j = $i + 1; $j < $pat_len; $j++) {
                        next unless substr($pat, $j, 1) eq ']';
                        last if $j - $i < 2;
                        if (substr($pat, $j - 1, 1) eq $curchar) {
                            # Here, is a posix class
                            $modified .= substr($pat, $i, $j - $i + 1) . " ";
                            $i = $j;
                            next CHAR;
                        }
                    }
                }

                # Here wasn't a posix class, just process normally
                $modified .= " $curchar ";
            }

            if ($in_brackets && ! $skip) {
                if ($result eq 'c') {
                    $skip++;
                    $reason = "Can't figure out where to put the (?[ and ]) since is a compilation error";
                }
                else {
                    print "not ok $testname # Problem in $0; original = '$pat'; mod = '$modified'\n";
                    next TEST;
                }
            }

            # Use our modified pattern instead of the original
            $pat = $modified;
        }
    }
    if ($::normalize_pat){
        if (!$skip && ($result eq "y" or $result eq "n")) {
            my $opat= $pat;
            # Convert (x)? to (?:(x)|) and (x)+ to (?:(x))+ and (x)* to (?:(x))*
            $pat =~ s/\(([\w|.]+)\)\?(?![+*?])/(?:($1)|)/g;
            $pat =~ s/\(([\w|.]+)\)([+*])(?![+*?])/(?:($1))$2/g;
            # inject an EVAL into the front of the pattern.
            # this should disable all optimizations.
            $pat =~ s/\A(.)/$1(?{ \$the_counter++ })/
                or die $pat;
        } elsif (!$skip) {
            $skip = $reason = "Test not applicable to $0";
        }
    }

    for my $study ('', 'study $subject;', 'utf8::upgrade($subject);',
		   'utf8::upgrade($subject); study $subject;') {
        if ( $skip ) {
            print "ok $testname # skipped", length($reason) ? ".  $reason" : '', "\n";
            next TEST;
        }
        our $the_counter = 0; # used in normalization tests
	# Need to make a copy, else the utf8::upgrade of an already studied
	# scalar confuses things.
	my $subject = $subject;
	$subject = XS::APItest::string_without_null($subject) if $no_null;
	my $c = $iters;
	my ($code, $match, $got);
        if ($repl eq 'pos') {
            my $patcode = defined $no_null_pat ? '/$no_null_pat/g'
                                               : "m${pat}g";
            $code= <<EOFCODE;
                $study
                pos(\$subject)=0;
                \$match = ( \$subject =~ $patcode );
                \$got = pos(\$subject);
EOFCODE
        }
        elsif ($qr_embed) {
            $code= <<EOFCODE;
                my \$RE = qr$pat;
                $study
                \$match = (\$subject =~ /(?:)\$RE(?:)/) while \$c--;
                \$got = "$repl";
EOFCODE
        }
        elsif ($qr_embed_thr) {
            $code= <<EOFCODE;
		# Can't run the match in a subthread, but can do this and
	 	# clone the pattern the other way.
                my \$RE = threads->new(sub {qr$pat})->join();
                $study
                \$match = (\$subject =~ /(?:)\$RE(?:)/) while \$c--;
                \$got = "$repl";
EOFCODE
        }
        elsif ($no_null) {
            my $patcode = defined $no_null_pat ? '/$no_null_pat/'
                                               :  $pat;
            $code= <<EOFCODE;
                $study
                \$match = (\$subject =~ $OP$pat) while \$c--;
                \$got = "$repl";
EOFCODE
        }
        else {
            $code= <<EOFCODE;
                $study
                \$match = (\$subject =~ $OP$pat) while \$c--;
                \$got = "$repl";
EOFCODE
        }
        $code = "$code" if $regex_sets;
        #$code.=qq[\n\$expect="$expect";\n];
        #use Devel::Peek;
        #die Dump($code) if $pat=~/\\h/ and $subject=~/\x{A0}/;
	{
	    # Probably we should annotate specific tests with which warnings
	    # categories they're known to trigger, and hence should be
	    # disabled just for that test
	    no warnings qw(uninitialized regexp deprecated);
	    eval $code;
	}
	chomp( my $err = $@ );
	if ($result eq 'c') {
	    if ($err !~ m!^\Q$expect!) { print "not ok $testname$todo (compile) $input => '$err'\n"; next TEST }
	    last;  # no need to study a syntax error
	}
	elsif ( $todo_qr ) {
	    print "not ok $testname # TODO", length($reason) ? " - $reason" : '', "\n";
	    next TEST;
	}
	elsif ($@) {
	    print "not ok $testname$todo $input => error '$err'\n", _comment("$code\n$@\n"); next TEST;
	}
	elsif ($result =~ /^n/) {
	    if ($match) { print "not ok $testname$todo ($study) $input => false positive\n"; next TEST }
	}
	else {
	    if (!$match || $got ne $expect) {
	        eval { require Data::Dumper };
                no warnings "utf8"; # But handle should be utf8
		if ($@ || !defined &DynaLoader::boot_DynaLoader) {
		    # Data::Dumper will load on miniperl, but fail when used in
		    # anger as it tries to load B. I'd prefer to keep the
		    # regular calls below outside of an eval so that real
		    # (unknown) failures get spotted, not ignored.
		    print "not ok $testname$todo ($study) $input => '$got', match=$match\n", _comment("$code\n");
		}
		else { # better diagnostics
		    my $s = Data::Dumper->new([$subject],['subject'])->Useqq(1)->Dump;
		    my $g = Data::Dumper->new([$got],['got'])->Useqq(1)->Dump;
		    my $e = Data::Dumper->new([$expect],['expected'])->Useqq(1)->Dump;
		    print "not ok $testname$todo ($study) $input => '$got', match=$match\n", _comment("$s\n$code\n$g\n$e\n");
		}
		next TEST;
	    }
	}
    }
    print "ok $testname$todo\n";
}

printf "1..%d\n# $iters iterations\n", scalar @tests;

1;
