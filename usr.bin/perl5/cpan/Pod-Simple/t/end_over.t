# head ends over
BEGIN {
    if($ENV{PERL_CORE}) {
        chdir 't';
        @INC = '../lib';
    }
}

use strict;
use warnings;
use Test;
BEGIN { plan tests => 7 };

BEGIN {
  require FindBin;
  unshift @INC, $FindBin::Bin . '/lib';
  require helpers;
  helpers->import('f');
}

my $d;
#use Pod::Simple::Debug (\$d,0);

ok 1;

use Pod::Simple::DumpAsXML;
use Pod::Simple::XMLOutStream;
print "# Pod::Simple version $Pod::Simple::VERSION\n";

sub nowhine {
  $_[0]->{'no_whining'} = 1;
}

&ok(f(
\&nowhine,
"=head2 BLOOP\n\nHoopbehwo!\n\n=over\n\n=item Stuff.  Um.\n\nBrop.\n\n=head1 SVUP\n\nMyup.",
"=head2 BLOOP\n\nHoopbehwo!\n\n=over\n\n=item Stuff.  Um.\n\nBrop.\n\n=back\n\n=head1 SVUP\n\nMyup.",
));

&ok(f(
\&nowhine,
"=head2 BLOOP\n\nHoopbehwo!\n\n=over\n\n=item Stuff.  Um.\n\nBrop.\n\n=head2 SVUP\n\nMyup.",
"=head2 BLOOP\n\nHoopbehwo!\n\n=over\n\n=item Stuff.  Um.\n\nBrop.\n\n=back\n\n=head2 SVUP\n\nMyup.",
));

&ok(f(
\&nowhine,
"=head2 BLOOP\n\nHoopbehwo!\n\n=over\n\n=item Stuff.  Um.\n\nBrop.\n\n=head3 SVUP\n\nMyup.",
"=head2 BLOOP\n\nHoopbehwo!\n\n=over\n\n=item Stuff.  Um.\n\nBrop.\n\n=back\n\n=head3 SVUP\n\nMyup.",
));

&ok(f(
\&nowhine,
"=head2 BLOOP\n\nHoopbehwo!\n\n=over\n\n=item Stuff.  Um.\n\nBrop.\n\n=head4 SVUP\n\nMyup.",
"=head2 BLOOP\n\nHoopbehwo!\n\n=over\n\n=item Stuff.  Um.\n\nBrop.\n\n=back\n\n=head4 SVUP\n\nMyup.",
));

&ok(f(
\&nowhine,
"=head2 BLOOP\n\nHoopbehwo!\n\n=over\n\n=item Stuff.  Um.\n\nBrop.\n\n=head5 SVUP\n\nMyup.",
"=head2 BLOOP\n\nHoopbehwo!\n\n=over\n\n=item Stuff.  Um.\n\nBrop.\n\n=back\n\n=head5 SVUP\n\nMyup.",
));

&ok(f(
\&nowhine,
"=head2 BLOOP\n\nHoopbehwo!\n\n=over\n\n=item Stuff.  Um.\n\nBrop.\n\n=head6 SVUP\n\nMyup.",
"=head2 BLOOP\n\nHoopbehwo!\n\n=over\n\n=item Stuff.  Um.\n\nBrop.\n\n=back\n\n=head6 SVUP\n\nMyup.",
));


__END__


