#!/usr/bin/env perl
use 5.010;
use open qw< :encoding(utf8) :std >;
use autodie;
use strict;
use File::Find;
use Encode::Guess;

# Check if POD files contain non-ASCII without specifying
# =encoding. Run it as:

## perl Porting/checkpodencoding.pl

find(
    {
        wanted => \&finder,
        no_chdir => 1,
    },
    '.'
);

sub finder {
    my $file = $_;

    return if -d $file or -B $file;

    open my $fh, '<', $file;

    #say STDERR "Checking $file";

    next if
        # Test cases
        $file =~ m[Pod-Simple/t];

    my ($in_pod, $has_encoding, @non_ascii);

    FILE: while (my $line = <$fh>) {
        chomp $line;
        if ($line =~ /^=[a-z]+/) {
            $in_pod = 1;
        }

        if ($in_pod) {
            if ($line =~ /^=encoding (\S+)/) {
                $has_encoding = 1;
                last FILE;
            } elsif ($line =~ /[^[:ascii:]]/) {
                my $encoding = guess_encoding($line);
                push @non_ascii => {
                    num => $.,
                    line => $line,
                    encoding => (ref $encoding ? "$encoding->{Name}?" : 'unknown!'),
                };
            }
        }

        if ($line =~ /^=cut/) {
            $in_pod = 0;
        }
    }

    if (@non_ascii and not $has_encoding) {
        say "$file:";
        $DB::single = 1;
        for (@non_ascii) {
            say "    $_->{num} ($_->{encoding}): $_->{line}";
        }
    }
}
