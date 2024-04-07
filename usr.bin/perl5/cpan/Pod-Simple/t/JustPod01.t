#! user/bin/perl -w

# t/JustPod01.t - check basics of Pod::Simple::JustPod

BEGIN {
  chdir 't' if -d 't';
}

use strict;
use lib '../lib';
use Test::More tests => 2;

use warnings;
use utf8;

use_ok('Pod::Simple::JustPod') or exit;

my $parser = Pod::Simple::JustPod->new();

my $input;
while ( <DATA> ) { $input .= $_ }

my $output;
$parser->output_string( \$output );
$parser->parse_string_document( $input );

# Strip off text before =pod in the input
$input =~ s/^.*(=pod.*)$/$1/mgs;

my $msg = "got expected output";
if ($output eq $input) {
    pass($msg);
}
elsif ($ENV{PERL_TEST_DIFF}) {
    fail($msg);
    require File::Temp;
    my $orig_file = File::Temp->new();
    local $/ = "\n";
    chomp $input;
    print $orig_file $input, "\n";
    close $orig_file || die "Can't close orig_file: $!";

    chomp $output;
    my $parsed_file = File::Temp->new();
    print $parsed_file $output, "\n";
    close $parsed_file || die "Can't close parsed_file";

    my $diff = File::Temp->new();
    system("$ENV{PERL_TEST_DIFF} $orig_file $parsed_file > $diff");

    open my $fh, "<", $diff || die "Can't open $diff";
    my @diffs = <$fh>;
    diag(@diffs);
}
else {
    eval { require Text::Diff; };
    if ($@) {
        is($output, $input, $msg);
        diag("Set environment variable PERL_TEST_DIFF=diff_tool or install"
           . " Text::Diff to see just the differences.");
    }
    else {
        fail($msg);
        diag Text::Diff::diff(\$input, \$output, { STYLE => 'Unified' });
    }
}


__DATA__
package utf8::all;
use strict;
use warnings;
use 5.010; # state
# ABSTRACT: turn on Unicode - all of it
our $VERSION = '0.010'; # VERSION


use Import::Into;
use parent qw(Encode charnames utf8 open warnings feature);

sub import {
    my $target = caller;
    'utf8'->import::into($target);
    'open'->import::into($target, qw{:encoding(UTF-8) :std});
    'charnames'->import::into($target, qw{:full :short});
    'warnings'->import::into($target, qw{FATAL utf8});
    'feature'->import::into($target, qw{unicode_strings}) if $^V >= v5.11.0;
    'feature'->import::into($target, qw{unicode_eval fc}) if $^V >= v5.16.0;

    {
        no strict qw(refs); ## no critic (TestingAndDebugging::ProhibitNoStrict)
        *{$target . '::readdir'} = \&_utf8_readdir;
    }

    # utf8 in @ARGV
    state $have_encoded_argv = 0;
    _encode_argv() unless $have_encoded_argv++;

    $^H{'utf8::all'} = 1;

    return;
}

sub _encode_argv {
    $_ = Encode::decode('UTF-8', $_) for @ARGV;
    return;
}

sub _utf8_readdir(*) { ## no critic (Subroutines::ProhibitSubroutinePrototypes)
    my $handle = shift;
    if (wantarray) {
        my @all_files  = CORE::readdir($handle);
        $_ = Encode::decode('UTF-8', $_) for @all_files;
        return @all_files;
    }
    else {
        my $next_file = CORE::readdir($handle);
        $next_file = Encode::decode('UTF-8', $next_file);
        return $next_file;
    }
}


1;

__END__

=pod

=encoding utf-8

=head1 NAME

utf8::all - turn on Unicode - all of it

=head1 VERSION

version 0.010

=head1 SYNOPSIS

    use utf8::all; # Turn on UTF-8. All of it.

    open my $in, '<', 'contains-utf8';  # UTF-8 already turned on here
    print length 'føø bār';             # 7 UTF-8 characters
    my $utf8_arg = shift @ARGV;         # @ARGV is UTF-8 too!

=head1 DESCRIPTION

L<utf8> allows you to write your Perl encoded in UTF-8. That means UTF-8
strings, variable names, and regular expressions. C<utf8::all> goes further, and
makes C<@ARGV> encoded in UTF-8, and filehandles are opened with UTF-8 encoding
turned on by default (including STDIN, STDOUT, STDERR), and charnames are
imported so C<\N{...}> sequences can be used to compile Unicode characters based
on names. If you I<don't> want UTF-8 for a particular filehandle, you'll have to
set C<binmode $filehandle>.

The pragma is lexically-scoped, so you can do the following if you had some
reason to:

    {
        use utf8::all;
        open my $out, '>', 'outfile';
        my $utf8_str = 'føø bār';
        print length $utf8_str, "\n"; # 7
        print $out $utf8_str;         # out as utf8
    }
    open my $in, '<', 'outfile';      # in as raw
    my $text = do { local $/; <$in>};
    print length $text, "\n";         # 10, not 7!

=head1 INTERACTION WITH AUTODIE

If you use L<autodie>, which is a great idea, you need to use at least version
B<2.12>, released on L<June 26, 2012|https://metacpan.org/source/PJF/autodie-2.12/Changes#L3>.
Otherwise, autodie obliterates the IO layers set by the L<open> pragma. See
L<RT #54777|https://rt.cpan.org/Ticket/Display.html?id=54777> and
L<GH #7|https://github.com/doherty/utf8-all/issues/7>.

=head1 AVAILABILITY

The project homepage is L<http://metacpan.org/release/utf8-all/>.

The latest version of this module is available from the Comprehensive Perl
Archive Network (CPAN). Visit L<http://www.perl.com/CPAN/> to find a CPAN
site near you, or see L<https://metacpan.org/module/utf8::all/>.

=head1 SOURCE

The development version is on github at L<http://github.com/doherty/utf8-all>
and may be cloned from L<git://github.com/doherty/utf8-all.git>

=head1 BUGS AND LIMITATIONS

You can make new bug reports, and view existing ones, through the
web interface at L<https://github.com/doherty/utf8-all/issues>.

=head1 AUTHORS

=over 4

=item *

Michael Schwern <mschwern@cpan.org>

=item *

Mike Doherty <doherty@cpan.org>

=back

=head1 COPYRIGHT AND LICENSE

This software is copyright (c) 2009 by Michael Schwern <mschwern@cpan.org>.

This is free software; you can redistribute it and/or modify it under
the same terms as the Perl 5 programming language system itself.

=cut
