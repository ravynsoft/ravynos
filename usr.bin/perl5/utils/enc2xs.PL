#!/usr/local/bin/perl

# Copy the cpan/Encode/bin/enc2xs script to utils/, while
# prepending a suitable #! invocation.

use Config;
use File::Basename qw(&basename &dirname);
use Cwd;

# List explicitly here the variables you want Configure to
# generate.  Metaconfig only looks for shell variables, so you
# have to mention them as if they were shell variables, not
# %Config entries.  Thus you write
#  $startperl
# to ensure Configure will look for $Config{startperl}.

# This forces PL files to create target in same directory as PL file.
# This is so that make depend always knows where to find PL derivatives.
my $origdir = cwd;
chdir dirname($0);
my $file = basename($0, '.PL');
$file .= '.com' if $^O eq 'VMS';

open OUT, '>', $file or die "Can't create $file: $!";

print "Extracting $file (with variable substitutions)\n";

# In this section, perl variables will be expanded during extraction.
# You can use $Config{...} to use Configure variables.

print OUT <<"!GROK!THIS!";
$Config{startperl}
    eval 'exec $Config{perlpath} -S \$0 \${1+"\$@"}'
	if 0; # ^ Run only under a shell
!GROK!THIS!

use File::Spec;

my $enc2xs = File::Spec->catfile(File::Spec->catdir(File::Spec->updir, "cpan", "Encode", "bin"), "enc2xs");

if (open(ENC2XS, '<', $enc2xs)) {
    print OUT <ENC2XS>;
    close ENC2XS;
} else {
    die "$0: cannot find '$enc2xs'\n";
}

close OUT or die "Can't close $file: $!";
chmod 0755, $file or die "Can't reset permissions for $file: $!\n";
exec("$Config{'eunicefix'} $file") if $Config{'eunicefix'} ne ':';
chdir $origdir;
