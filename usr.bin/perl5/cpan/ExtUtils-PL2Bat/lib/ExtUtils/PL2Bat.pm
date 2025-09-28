package ExtUtils::PL2Bat;
$ExtUtils::PL2Bat::VERSION = '0.005';
use strict;
use warnings;

use 5.006;

use Config;
use Carp qw/croak/;

# In core, we can't use any other modules except those that already live in
# lib/, so Exporter is not available to us.
sub import {
	my ($self, @functions) = @_;
	@functions = 'pl2bat' if not @functions;
	my $caller = caller;
	for my $function (@functions) {
		no strict 'refs';
		*{"$caller\::$function"} = \&{$function};
	}
}

sub pl2bat {
	my %opts = @_;

	# NOTE: %0 is already enclosed in doublequotes by cmd.exe, as appropriate
	$opts{ntargs}    = '-x -S %0 %*' unless exists $opts{ntargs};
	$opts{otherargs} = '-x -S "%0" %1 %2 %3 %4 %5 %6 %7 %8 %9' unless exists $opts{otherargs};

	$opts{stripsuffix} = qr/\.plx?/ unless exists $opts{stripsuffix};

	if (not exists $opts{out}) {
		$opts{out} = $opts{in};
		$opts{out} =~ s/$opts{stripsuffix}$//i;
		$opts{out} .= '.bat' unless $opts{in} =~ /\.bat$/i or $opts{in} eq '-';
	}

	my $head = <<"EOT";
	\@rem = '--*-Perl-*--
	\@set "ErrorLevel="
	\@if "%OS%" == "Windows_NT" \@goto WinNT
	\@perl $opts{otherargs}
	\@set ErrorLevel=%ErrorLevel%
	\@goto endofperl
	:WinNT
	\@perl $opts{ntargs}
	\@set ErrorLevel=%ErrorLevel%
	\@if NOT "%COMSPEC%" == "%SystemRoot%\\system32\\cmd.exe" \@goto endofperl
	\@if %ErrorLevel% == 9009 \@echo You do not have Perl in your PATH.
	\@goto endofperl
	\@rem ';
EOT

	$head =~ s/^\s+//gm;
	my $headlines = 2 + ($head =~ tr/\n/\n/);
	my $tail = <<'EOT';
	__END__
	:endofperl
	@set "ErrorLevel=" & @goto _undefined_label_ 2>NUL || @"%COMSPEC%" /d/c @exit %ErrorLevel%
EOT
	$tail =~ s/^\s+//gm;

	my $linedone = 0;
	my $taildone = 0;
	my $linenum = 0;
	my $skiplines = 0;

	my $start = $Config{startperl};
	$start = '#!perl' unless $start =~ /^#!.*perl/;

	open my $in, '<', $opts{in} or croak "Can't open $opts{in}: $!";
	my @file = <$in>;
	close $in;

	foreach my $line ( @file ) {
		$linenum++;
		if ( $line =~ /^:endofperl\b/ ) {
			if (!exists $opts{update}) {
				warn "$opts{in} has already been converted to a batch file!\n";
				return;
			}
			$taildone++;
		}
		if ( not $linedone and $line =~ /^#!.*perl/ ) {
			if (exists $opts{update}) {
				$skiplines = $linenum - 1;
				$line .= '#line '.(1+$headlines)."\n";
			} else {
	$line .= '#line '.($linenum+$headlines)."\n";
			}
	$linedone++;
		}
		if ( $line =~ /^#\s*line\b/ and $linenum == 2 + $skiplines ) {
			$line = '';
		}
	}

	open my $out, '>', $opts{out} or croak "Can't open $opts{out}: $!";
	print $out $head;
	print $out $start, ( $opts{usewarnings} ? ' -w' : '' ),
						 "\n#line ", ($headlines+1), "\n" unless $linedone;
	print $out @file[$skiplines..$#file];
	print $out $tail unless $taildone;
	close $out;

	return $opts{out};
}

1;

# ABSTRACT: Batch file creation to run perl scripts on Windows

__END__

=pod

=encoding UTF-8

=head1 NAME

ExtUtils::PL2Bat - Batch file creation to run perl scripts on Windows

=head1 VERSION

version 0.005

=head1 OVERVIEW

This module converts a perl script into a batch file that can be executed on Windows/DOS-like operating systems.  This is intended to allow you to use a Perl script like regular programs and batch files where you just enter the name of the script [probably minus the extension] plus any command-line arguments and the script is found in your B<PATH> and run.

=head1 FUNCTIONS

=head2 pl2bat(%opts)

This function takes a perl script and write a batch file that contains the script. This is sometimes necessary

=over 8

=item * C<in>

The name of the script that is to be batchified. This argument is mandatory.

=item * C<out>

The name of the output batch file. If not given, it will be generated using C<in> and C<stripsuffix>.

=item * C<ntargs>

Arguments to invoke perl with in generated batch file when run from
Windows NT.  Defaults to S<'-x -S %0 %*'>.

=item * C<otherargs>

Arguments to invoke perl with in generated batch file except when
run from Windows NT (ie. when run from DOS, Windows 3.1, or Windows 95).
Defaults to S<'-x -S "%0" %1 %2 %3 %4 %5 %6 %7 %8 %9'>.

=item * C<stripsuffix>

Strip a suffix string from file name before appending a ".bat"
suffix.  The suffix is not case-sensitive.  It can be a regex or a string and a trailing
C<$> is always assumed).  Defaults to C<qr/\.plx?/>.

=item * C<usewarnings>

With the C<usewarnings>
option, C<" -w"> is added after the value of C<$Config{startperl}>.
If a line matching C</^#!.*perl/> already exists in the script,
then it is not changed and the B<-w> option is ignored.

=item * C<update>

If the script appears to have already been processed by B<pl2bat>,
then the script is skipped and not processed unless C<update> was
specified.  If C<update> is specified, the existing preamble is replaced.

=back

=head1 ACKNOWLEDGEMENTS

This code was taken from Module::Build and then modified; which had taken it from perl's pl2bat script. This module is an attempt at unifying all three implementations.

=head1 AUTHOR

Leon Timmermans <leont@cpan.org>

=head1 COPYRIGHT AND LICENSE

This software is copyright (c) 2015 by Leon Timmermans.

This is free software; you can redistribute it and/or modify it under
the same terms as the Perl 5 programming language system itself.

=cut
