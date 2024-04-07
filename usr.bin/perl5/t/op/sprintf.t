#!./perl

# Tests sprintf, excluding handling of 64-bit integers or long
# doubles (if supported), of machine-specific short and long
# integers, machine-specific floating point exceptions (infinity,
# not-a-number ...), of the effects of locale, and of features
# specific to multi-byte characters (under the utf8 pragma and such).

# For tests that do not fit this format, use sprintf2.t.

BEGIN {
    chdir 't' if -d 't';
    require './test.pl';
    set_up_inc(qw '../lib ../cpan/version/lib');
}
use warnings;
use version;
use Config;
use strict;


my @tests = ();
my ($template, $data, $result, $comment, $w, $x, $evalData, $n, $p);

my $Is_VMS_VAX = 0;
# We use HW_MODEL since ARCH_NAME was not in VMS V5.*
if ($^O eq 'VMS') {
    my $hw_model;
    chomp($hw_model = `write sys\$output f\$getsyi("HW_MODEL")`);
    $Is_VMS_VAX = $hw_model < 1024 ? 1 : 0;
}

# The most generic VAX catcher.
my $Is_VAX_Float = (pack("d", 1) =~ /^[\x80\x10]\x40/);

our $IS_EBCDIC = $::IS_EBCDIC;  # Solely to avoid the 'used once' warning
our $IS_ASCII = $::IS_ASCII;   # Solely to avoid the 'used once' warning

while (<DATA>) {
    s/<\s*$//;

    # An initial 'a' or 'e' marks the test as being only for ASCII or EBCDIC
    # platforms respectively.
    s/^\s* ( [ae] )? >//x;
    next if defined $1 && $1 eq 'a' && $::IS_EBCDIC;
    next if defined $1 && $1 eq 'e' && $::IS_ASCII;

    ($template, $data, $result, $comment) = split(/<\s*>/, $_, 4);
    if ($^O eq 'os390' || $^O eq 's390') { # non-IEEE (s390 is UTS)
        $data   =~ s/([eE])96$/${1}63/;      # smaller exponents
        $result =~ s/([eE]\+)102$/${1}69/;   #  "       "
        $data   =~ s/([eE])\-101$/${1}-56/;  # larger exponents
        $result =~ s/([eE])\-102$/${1}-57/;  #  "       "
    }
    if ($Is_VMS_VAX || $Is_VAX_Float) {
	# VAX DEC C 5.3 at least since there is no
	# ccflags =~ /float=ieee/ on VAX.
	# AXP is unaffected whether or not it is using ieee.
        $data   =~ s/([eE])96$/${1}26/;      # smaller exponents
        $result =~ s/([eE]\+)102$/${1}32/;   #  "       "
        $data   =~ s/([eE])\-101$/${1}-24/;  # larger exponents
        $result =~ s/([eE])\-102$/${1}-25/;  #  "       "
    }

    $evalData = eval $data;
    $evalData = ref $evalData ? $evalData : [$evalData];
    push @tests, [$template, $evalData, $result, $comment, $data];
}

plan(scalar @tests);

$SIG{__WARN__} = sub {
    if ($_[0] =~ /^Invalid conversion/) {
	$w .= ' INVALID';
    } elsif ($_[0] =~ /^Use of uninitialized value/) {
	$w .= ' UNINIT';
    } elsif ($_[0] =~ /^Missing argument/) {
	$w .= ' MISSING';
    } elsif ($_[0] =~ /^Redundant argument/) {
	$w .= ' REDUNDANT';
    } elsif ($_[0]=~/^vector argument not supported with alpha versions/) {
	$w .= ' ALPHA';
    } else {
	warn @_;
    }
};

for (@tests) {
    ($template, $evalData, $result, $comment, $data) = @$_;
    $w = undef;
    $x = sprintf($template, @$evalData);
    $x = ">$x<" if defined $x;
    substr($x, -1, 0) = $w if $w;
    # $x may have 3 exponent digits, not 2
    my $y = $x;
    if ($y =~ s/([Ee][-+])0(\d)/$1$2/) {
        # if result is left-adjusted, append extra space
        if ($template =~ /%\+?\-/ and $result =~ / $/) {
	    $y =~ s/<$/ </;
	}
        # if result is zero-filled, add extra zero
	elsif ($template =~ /%\+?0/ and $result =~ /^0/) {
	    $y =~ s/^>0/>00/;
	}
        # if result is right-adjusted, prepend extra space
	elsif ($result =~ /^ /) {
	    $y =~ s/^>/> /;
	}
    }

    my $skip = 0;
    if ($comment =~ s/\s+skip:\s*(.*)//) {
	my $os  = $1;
	my $osv = exists $Config{osvers} ? $Config{osvers} : "0";
	my $archname = $Config{archname};
	# >comment skip: all<
	# >comment skip: solaris<
        # >comment skip: x86_64-linux-ld<
	if ($os =~ /\b(?:all|\Q$^O\E|\Q$archname\E)\b/i) {
            $skip = 1;
	} elsif ($os =~ /\b\Q$^O\E(?::(\S+))\b/i) {
            # We can have the $^O followed by an optional condition.
            # The condition, if present, can be one of:
            # (1) starts with a digit...
            #     the first pair of dot-separated digits is
            #     tested numerically against $Config{osvers}
            # (2) otherwise...
            #     tested as a \b/i regex against $Config{archname}
            my $cond = $1;
            if ($cond =~ /^\d/) {
                # >comment skip: hpux:10.20<
                my $vsn = $cond;
                # Only compare on the first pair of digits, as numeric
                # compares do not like 2.6.10-3mdksmp or 2.6.8-24.10-default
                s/^(\d+(\.\d+)?).*/$1/ for $osv, $vsn;
                $skip = $vsn ? ($osv <= $vsn ? 1 : 0) : 1;
            } else {
                # >comment skip: netbsd:vax-netbsd<
                $skip = $archname =~ /\b\Q$cond\E\b/i;
            }
	}
	$skip and $comment =~ s/$/, failure expected on $^O $osv $archname/;
    }

    if ($x eq ">$result<") {
        ok(1, join ' ', grep length, ">$result<", $comment);
    }
    elsif ($skip) {
      SKIP: { skip($comment, 1) }
    }
    elsif ($y eq ">$result<")	# Some C libraries always give
    {				# three-digit exponent
		ok(1, ">$result< $x three-digit exponent accepted");
    }
	elsif ($result =~ /[-+]\d{3}$/ &&
		   # Suppress tests with modulo of exponent >= 100 on platforms
		   # which cannot handle such magnitudes (or where we cannot tell).
		   ((!eval {require POSIX}) || # Costly: only do this if we must!
			(length(&POSIX::DBL_MAX) - rindex(&POSIX::DBL_MAX, '+')) == 3))
	{
        ok(1,
         ">$template< >$data< >$result< Suppressed: exponent out of range?\n");
	}
    else {
        $y = ($x eq $y ? "" : " => $y");
        ok(0, ">$template< >$data< >$result< $x$y $comment");
    }
}

# In each of the following lines, there are three required fields:
# printf template, data to be formatted (as a Perl expression), and
# expected result of formatting.  An optional fourth field can contain
# a comment.  Each field is delimited by a starting '>' and a
# finishing '<'; any whitespace outside these start and end marks is
# not part of the field.  If formatting requires more than one data
# item (for example, if variable field widths are used), the Perl data
# expression should return a reference to an array having the requisite
# number of elements.  Even so, subterfuge is sometimes required: see
# tests for %n and %p.
#
# Tests that are expected to fail on a certain OS can be marked as such
# by trailing the comment with a skip: section. Skips are tags separated
# by space consisting of a $^O optionally trailed with :osvers or :archname.
# In the osvers case, all os-levels below that are expected to fail.
# In the archname case, an exact match is expected, unless the archname
# begins (and ends) with a "/", in which case a regexp is expected.
# A special tag 'all' is allowed for todo tests that should fail on any system
#
# >%G<   >1234567e96<  >1.23457E+102<   >exponent too big skip: os390<
# >%.0g< >-0.0<        >-0<             >No minus skip: MSWin32 VMS hpux:10.20<
# >%d<   >4<           >1<              >4 != 1 skip: all<
#
# The following tests are not currently run, for the reasons stated:

=pod

=begin problematic

>%.0f<      >1.5<         >2<   >Standard vague: no rounding rules<
>%.0f<      >2.5<         >2<   >Standard vague: no rounding rules<

=end problematic

=cut

# template    data          result
__END__
>%6. 6s<    >''<          >%6. 6s INVALID< >(See use of $w in code above)<
>%6 .6s<    >''<          >%6 .6s INVALID<
>%6.6 s<    >''<          >%6.6 s INVALID<
>%A<        >0<           ><	 >%A tested in sprintf2.t skip: all<
>%B<        >2**32-1<     >11111111111111111111111111111111<
>%+B<       >2**32-1<     >11111111111111111111111111111111<
>%#B<       >2**32-1<     >0B11111111111111111111111111111111<
>%C<        >''<          >%C INVALID<
>%D<        >0x7fffffff<  >2147483647<     >Synonym for %ld<
>%E<        >123456.789<  >1.234568E+05<   >Like %e, but using upper-case "E"<
>%F<        >123456.789<  >123456.789000<  >Synonym for %f<
>%G<        >1234567.89<  >1.23457E+06<    >Like %g, but using upper-case "E"<
>%G<        >1234567e96<  >1.23457E+102<
>%G<        >.1234567e-101< >1.23457E-102<
>%G<        >12345.6789<  >12345.7<
>%G<        >1234567e96<  >1.23457E+102<	>exponent too big skip: os390<
>%G<        >.1234567e-101< >1.23457E-102<	>exponent too small skip: os390<
>%H<        >''<          >%H INVALID<
>%I<        >''<          >%I INVALID<
>%J<        >''<          >%J INVALID<
>%K<        >''<          >%K INVALID<
>%L<        >''<          >%L INVALID<
>%M<        >''<          >%M INVALID<
>%N<        >''<          >%N INVALID<
>%O<        >2**32-1<     >37777777777<    >Synonym for %lo<
>%P<        >''<          >%P INVALID<
>%Q<        >''<          >%Q INVALID<
>%R<        >''<          >%R INVALID<
>%S<        >''<          >%S INVALID<
>%T<        >''<          >%T INVALID<
>%U<        >2**32-1<     >4294967295<     >Synonym for %lu<
>%V<        >''<          >%V INVALID<
>%W<        >''<          >%W INVALID<
>%X<        >2**32-1<     >FFFFFFFF<       >Like %x, but with u/c letters<
>%#X<       >2**32-1<     >0XFFFFFFFF<
>%Y<        >''<          >%Y INVALID<
>%Z<        >''<          >%Z INVALID<
>%a<        >0<           ><	 >%a tested in sprintf2.t skip: all<
>%b<        >2**32-1<     >11111111111111111111111111111111<
>%+b<       >2**32-1<     >11111111111111111111111111111111<
>%#b<       >2**32-1<     >0b11111111111111111111111111111111<
>%34b<      >2**32-1<     >  11111111111111111111111111111111<
>%034b<     >2**32-1<     >0011111111111111111111111111111111<
>%-34b<     >2**32-1<     >11111111111111111111111111111111  <
>%-034b<    >2**32-1<     >11111111111111111111111111111111  <
>%6b<       >12<          >  1100<
>%6.5b<     >12<          > 01100<
>%-6.5b<    >12<          >01100 <
>%+6.5b<    >12<          > 01100<
>% 6.5b<    >12<          > 01100<
>%06.5b<    >12<          > 01100<         >0 flag with precision: no effect<
>%.5b<      >12<          >01100<
>%.0b<      >0<           ><
>%+.0b<     >0<           ><
>% .0b<     >0<           ><
>%-.0b<     >0<           ><
>%#.0b<     >0<           ><
>%#3.0b<    >0<           >   <
>%#3.1b<    >0<           >  0<
>%#3.2b<    >0<           > 00<
>%#3.3b<    >0<           >000<
>%#3.4b<    >0<           >0000<
>%.0b<      >1<           >1<
>%+.0b<     >1<           >1<
>% .0b<     >1<           >1<
>%-.0b<     >1<           >1<
>%#.0b<     >1<           >0b1<
>%#3.0b<    >1<           >0b1<
>%#3.1b<    >1<           >0b1<
>%#3.2b<    >1<           >0b01<
>%#3.3b<    >1<           >0b001<
>%#3.4b<    >1<           >0b0001<
>%c<        >ord('A')<    >A<
>%10c<      >ord('A')<    >         A<
>%#10c<     >ord('A')<    >         A<     ># modifier: no effect<
>%010c<     >ord('A')<    >000000000A<
>%10lc<     >ord('A')<    >         A<     >l modifier: no effect<
>%10hc<     >ord('A')<    >         A<     >h modifier: no effect<
>%10.5c<    >ord('A')<    >         A<     >precision: no effect<
>%-10c<     >ord('A')<    >A         <
>%d<        >123456.789<  >123456<
>%d<        >-123456.789< >-123456<
>%d<        >0<           >0<
>%-d<       >0<           >0<
>%+d<       >0<           >+0<
>% d<       >0<           > 0<
>%0d<       >0<           >0<
>%-3d<      >1<           >1  <
>%+3d<      >1<           > +1<
>% 3d<      >1<           >  1<
>%03d<      >1<           >001<
>%+ 3d<     >1<           > +1<
>% +3d<     >1<           > +1<
>%.0d<      >0<           ><
>%+.0d<     >0<           >+<
>% .0d<     >0<           > <
>%-.0d<     >0<           ><
>%#.0d<     >0<           ><
>%.0d<      >1<           >1<
>%d<        >1<           >1<
>%+d<       >1<           >+1<
>%#3.2d<    >1<           > 01<            ># modifier: no effect<
>%3.2d<     >1<           > 01<
>%03.2d<    >1<           > 01<            >0 flag with precision: no effect<
>%-3.2d<    >1<           >01 <
>%+3.2d<    >1<           >+01<
>% 3.2d<    >1<           > 01<
>%-03.2d<   >1<           >01 <            >zero pad + left just.: no effect<
>%3.*d<     >[2,1]<       > 01<
>%3.*d<     >[1,1]<       >  1<
>%3.*d<     >[0,1]<       >  1<
>%3.*d<     >[-1,1]<      >  1<
>%.*d<      >[0,0]<       ><
>%-.*d<     >[0,0]<       ><
>%+.*d<     >[0,0]<       >+<
>% .*d<     >[0,0]<       > <
>%0.*d<     >[0,0]<       ><
>%.*d<      >[-2,0]<      >0<
>%-.*d<     >[-2,0]<      >0<
>%+.*d<     >[-2,0]<      >+0<
>% .*d<     >[-2,0]<      > 0<
>%0.*d<     >[-2,0]<      >0<
>%.*2$d<    >[5,3]<       >005<           >reordered precision arg<
>%4.*2$d<   >[5,3]<       > 005<          >width with reordered precision<
>%*3$.*2$d< >[5,3,4]<     > 005<          >reordered width with reordered precision<
>%3$*2$.*1$d< >[3,4,5]<   > 005<          >reordered param, width, precision<
>%*1$.*f<   >[4, 5, 10]<  >5.0000<        >perl #125956: reordered param, width, precision, floating point<
>%d<        >-1<          >-1<
>%-d<       >-1<          >-1<
>%+d<       >-1<          >-1<
>% d<       >-1<          >-1<
>%-3d<      >-1<          >-1 <
>%+3d<      >-1<          > -1<
>% 3d<      >-1<          > -1<
>%03d<      >-1<          >-01<
>%hd<       >1<           >1<              >More extensive testing of<
>%hhd<      >1<           >1<              >length modifiers would be<
>%ld<       >1<           >1<              >platform-specific<
>%Vd<       >1<           >1<
>%zd<       >1<           >1<
>%td<       >1<           >1<
>%vd<       >chr(1)<      >1<
>%+vd<      >chr(1)<      >+1<
>%#vd<      >chr(1)<      >1<
>%vd<       >"\01\02\03"< >1.2.3<
>%vd<       >v1.2.3<      >1.2.3<
>%vd<       >[version::qv("1.2.3")]< >1.2.3<
>%vd<       >[version->new("1.2")]< >1.2<
>%vd<       >[version->new("1.02")]< >1.2<
>%vd<       >[version->new("1.002")]< >1.2<
>%vd<       >[version->new("1048576.5")]< >1048576.5<
>%vd<       >[version->new("50")]< >50<
>%v.3d<     >"\01\02\03"< >001.002.003<
>%0v3d<     >"\01\02\03"< >001.002.003<
>%v.3d<     >[version::qv("1.2.3")]< >001.002.003<
>%-v3d<     >"\01\02\03"< >1  .2  .3  <
>%+-v3d<    >"\01\02\03"< >+1 .2  .3  <
>%+-v3d<    >[version::qv("1.2.3")]< >+1 .2  .3  <
>%v4.3d<    >"\01\02\03"< > 001. 002. 003<
>%0v4.3d<   >"\01\02\03"< > 001. 002. 003<
>%0*v2d<    >['-', "\0\7\14"]< >00-07-12<
>%v.*d<     >[3, "\01\02\03"]< >001.002.003< >cf perl #83194<
>%0v*d<     >[3, "\01\02\03"]< >001.002.003< >cf perl #83194<
>%-v*d<     >[3, "\01\02\03"]< >1  .2  .3  < >cf perl #83194<
>%+-v*d<    >[3, "\01\02\03"]< >+1 .2  .3  < >cf perl #83194<
>%v*.*d<    >[4, 3, "\01\02\03"]< > 001. 002. 003< >cf perl #83194<
>%0v*.*d<   >[4, 3, "\01\02\03"]< > 001. 002. 003< >cf perl #83194<
>%0*v*d<    >['-', 2, "\0\7\13"]< >00-07-11< >cf perl #83194<
>%0*v*d<    >['-', 2, version::qv("0.7.11")]< >00-07-11< >cf perl #83194<
>%e<        >1234.875<    >1.234875e+03<
>%e<        >0.000012345< >1.234500e-05<
>%e<        >1234567E96<  >1.234567e+102<
>%e<        >0<           >0.000000e+00<
>%e<        >.1234567E-101< >1.234567e-102<
>%+e<       >1234.875<    >+1.234875e+03<
>%#e<       >1234.875<    >1.234875e+03<
>%e<        >-1234.875<   >-1.234875e+03<
>%+e<       >-1234.875<   >-1.234875e+03<
>%#e<       >-1234.875<   >-1.234875e+03<
>%.0e<      >1234.875<    >1e+03<
>%#.0e<     >1234.875<    >1.e+03<
>%.0e<      >1.875<       >2e+00<
>%.0e<      >0.875<       >9e-01<
>%.*e<      >[0, 1234.875]< >1e+03<
>%.1e<      >1234.875<    >1.2e+03<
>%-12.4e<   >1234.875<    >1.2349e+03  <
>%12.4e<    >1234.875<    >  1.2349e+03<
>%+-12.4e<  >1234.875<    >+1.2349e+03 <
>%+12.4e<   >1234.875<    > +1.2349e+03<
>%+-12.4e<  >-1234.875<   >-1.2349e+03 <
>%+12.4e<   >-1234.875<   > -1.2349e+03<
>%e<        >1234567E96<  >1.234567e+102<	>exponent too big skip: os390<
>%e<        >.1234567E-101< >1.234567e-102<	>exponent too small skip: os390<
>%f<        >1234.875<    >1234.875000<
>%+f<       >1234.875<    >+1234.875000<
>%#f<       >1234.875<    >1234.875000<
>%f<        >-1234.875<   >-1234.875000<
>%+f<       >-1234.875<   >-1234.875000<
>%#f<       >-1234.875<   >-1234.875000<
>%6f<       >1234.875<    >1234.875000<
>%*f<       >[6, 1234.875]< >1234.875000<
>%.0f<      >-0.1<        >-0<  >C library bug: no minus skip: VMS<
>%.0f<      >1234.875<    >1235<
>%.1f<      >1234.875<    >1234.9<
>%-8.1f<    >1234.875<    >1234.9  <
>%8.1f<     >1234.875<    >  1234.9<
>%+-8.1f<   >1234.875<    >+1234.9 <
>%+8.1f<    >1234.875<    > +1234.9<
>%+-8.1f<   >-1234.875<   >-1234.9 <
>%+8.1f<    >-1234.875<   > -1234.9<
>%*.*f<     >[5, 2, 12.3456]< >12.35<
>%f<        >0<           >0.000000<
>%.0f<      >[]<          >0 MISSING<
> %.0f<     >[]<          > 0 MISSING<
>%.2f<      >[]<          >0.00 MISSING<
>%.2fC<      >[]<          >0.00C MISSING<
>%.0f<      >0<           >0<
>%.0f<      >2**38<       >274877906944<   >Should have exact int'l rep'n<
>%.0f<      >0.1<         >0<
>%.0f<      >0.6<         >1<              >Known to fail with (irix|nonstop-ux); -DHAS_LDBL_SPRINTF_BUG may fix<
>%.0f<      >-0.6<        >-1<             >Known to fail with (irix|nonstop-ux); -DHAS_LDBL_SPRINTF_BUG may fix<
>%.0f<      >1.6<         >2<
>%.0f<      >-1.6<        >-2<
>%.0f<      >1<           >1<
>%#.0f<     >1<           >1.<
>%.0lf<     >1<           >1<              >'l' should have no effect<
>%.0hf<     >1<           >%.0hf INVALID<  >'h' should be rejected<
>%g<        >12345.6789<  >12345.7<
>%+g<       >12345.6789<  >+12345.7<
>%#g<       >12345.6789<  >12345.7<
>%.0g<      >[]<          >0 MISSING<
> %.0g<     >[]<          > 0 MISSING<
>%.2g<      >[]<          >0 MISSING<
>%.2gC<      >[]<          >0C MISSING<
>%.0g<      >-0.0<        >-0<		   >C99 standard mandates minus sign but C89 does not skip: MSWin32 VMS netbsd:vax-netbsd hpux:10.20 openbsd netbsd:1.5 irix darwin freebsd:4.9 android<
>%.0g<      >12345.6789<  >1e+04<
>%#.0g<     >12345.6789<  >1.e+04<
>%.2g<      >12345.6789<  >1.2e+04<
>%.*g<      >[2, 12345.6789]< >1.2e+04<
>%.9g<      >12345.6789<  >12345.6789<
>%12.9g<    >12345.6789<  >  12345.6789<
>%012.9g<   >12345.6789<  >0012345.6789<
>%-12.9g<   >12345.6789<  >12345.6789  <
>%*.*g<     >[-12, 9, 12345.6789]< >12345.6789  <
>%-012.9g<  >12345.6789<  >12345.6789  <
>%g<        >-12345.6789< >-12345.7<
>%+g<       >-12345.6789< >-12345.7<
>%g<        >1234567.89<  >1.23457e+06<
>%+g<       >1234567.89<  >+1.23457e+06<
>%#g<       >1234567.89<  >1.23457e+06<
>%g<        >-1234567.89< >-1.23457e+06<
>%+g<       >-1234567.89< >-1.23457e+06<
>%#g<       >-1234567.89< >-1.23457e+06<
>%g<        >0.00012345<  >0.00012345<
>%g<        >0.000012345< >1.2345e-05<
>%g<        >1234567E96<  >1.23457e+102<
>%g<        >.1234567E-101< >1.23457e-102<
>%g<        >0<           >0<
>%13g<      >1234567.89<  >  1.23457e+06<
>%+13g<     >1234567.89<  > +1.23457e+06<
>%013g<     >1234567.89<  >001.23457e+06<
>%-13g<     >1234567.89<  >1.23457e+06  <
>%g<        >.1234567E-101< >1.23457e-102<	>exponent too small skip: os390<
>%g<        >1234567E96<  >1.23457e+102<	>exponent too big skip: os390<
>%h<        >''<          >%h INVALID<
>%i<        >123456.789<  >123456<         >Synonym for %d<
>%j<        >''<          >%j INVALID<
>%k<        >''<          >%k INVALID<
>%l<        >''<          >%l INVALID<
>%m<        >''<          >%m INVALID<
>%s< >sprintf('%%n%n %d', $n, $n)< >%n 2< >Slight sneakiness to test %n<
>%s< >$n="abc"; sprintf(' %n%s', substr($n,1,1), $n)< > a1c< >%n w/magic<
>%s< >no warnings; sprintf('%s%n', chr(256)x5, $n),$n< >5< >Unicode %n<
>%o<        >2**32-1<     >37777777777<
>%+o<       >2**32-1<     >37777777777<
>%#o<       >2**32-1<     >037777777777<
>%o<        >642<         >1202<          >check smaller octals across platforms<
>%+o<       >642<         >1202<
>% o<       >642<         >1202<
>%#o<       >642<         >01202<
>%4o<       >18<          >  22<
>%4.3o<     >18<          > 022<
>%-4.3o<    >18<          >022 <
>%+4.3o<    >18<          > 022<
>% 4.3o<    >18<          > 022<
>%04.3o<    >18<          > 022<          >0 flag with precision: no effect<
>%4.o<      >36<          >  44<
>%-4.o<     >36<          >44  <
>%+4.o<     >36<          >  44<
>% 4.o<     >36<          >  44<
>%04.o<     >36<          >  44<          >0 flag with precision: no effect<
>%.3o<      >18<          >022<
>%.0o<      >0<           ><
>%+.0o<     >0<           ><
>% .0o<     >0<           ><
>%-.0o<     >0<           ><
>%#.0o<     >0<           >0<
>%#3.0o<    >0<           >  0<
>%#3.1o<    >0<           >  0<
>%#3.2o<    >0<           > 00<
>%#3.3o<    >0<           >000<
>%#3.4o<    >0<           >0000<
>%.0o<      >1<           >1<
>%+.0o<     >1<           >1<
>% .0o<     >1<           >1<
>%-.0o<     >1<           >1<
>%#.0o<     >1<           >01<
>%#3.0o<    >1<           > 01<
>%#3.1o<    >1<           > 01<
>%#3.2o<    >1<           > 01<
>%#3.3o<    >1<           >001<
>%#3.4o<    >1<           >0001<
>%#.5o<     >012345<      >012345<
>%#.5o<     >012<         >00012<
>%#4o<      >17<          > 021<
>%#-4o<     >17<          >021 <
>%-#4o<     >17<          >021 <
>%#+4o<     >17<          > 021<
>%# 4o<     >17<          > 021<
>%#04o<     >17<          >0021<
>%#4.o<     >16<          > 020<
>%#-4.o<    >16<          >020 <
>%-#4.o<    >16<          >020 <
>%#+4.o<    >16<          > 020<
>%# 4.o<    >16<          > 020<
>%#04.o<    >16<          > 020<          >0 flag with precision: no effect<
>%#4.3o<    >18<          > 022<
>%#-4.3o<   >18<          >022 <
>%-#4.3o<   >18<          >022 <
>%#+4.3o<   >18<          > 022<
>%# 4.3o<   >18<          > 022<
>%#04.3o<   >18<          > 022<          >0 flag with precision: no effect<
>%#6.4o<    >18<          >  0022<
>%#-6.4o<   >18<          >0022  <
>%-#6.4o<   >18<          >0022  <
>%#+6.4o<   >18<          >  0022<
>%# 6.4o<   >18<          >  0022<
>%#06.4o<   >18<          >  0022<        >0 flag with precision: no effect<
>%d< >$p=sprintf('%p',$p);$p=~/^[0-9a-f]+$/< >1< >Coarse hack: hex from %p?<
>%d< >$p=sprintf('%-8p',$p);$p=~/^[0-9a-f]+\s*$/< >1< >Coarse hack: hex from %p?<
>%d< >$p=sprintf('%#p',$p);$p=~/^0x[0-9a-f]+\s*$/< >1< >Coarse hack: hex from %#p<
>%q<        >''<          >%q INVALID<
>%r<        >''<          >%r INVALID<
>%s<        >[]<          > MISSING<
> %s<       >[]<          >  MISSING<
>%s<        >'string'<    >string<
>%10s<      >'string'<    >    string<
>%+10s<     >'string'<    >    string<
>%#10s<     >'string'<    >    string<
>%010s<     >'string'<    >0000string<
>%0*s<      >[10, 'string']< >0000string<
>%-10s<     >'string'<    >string    <
>%3s<       >'string'<    >string<
>%.3s<      >'string'<    >str<
>%.*s<      >[3, 'string']< >str<
>%.*s<      >[2, 'string']< >st<
>%.*s<      >[1, 'string']< >s<
>%.*s<      >[0, 'string']< ><
>%.*s<      >[-1,'string']< >string<  >negative precision to be ignored<
>%3.*s<     >[3, 'string']< >str<
>%3.*s<     >[2, 'string']< > st<
>%3.*s<     >[1, 'string']< >  s<
>%3.*s<     >[0, 'string']< >   <
>%3.*s<     >[-1,'string']< >string<  >negative precision to be ignored<
>%t<        >''<          >%t INVALID<
>%u<        >2**32-1<     >4294967295<
>%+u<       >2**32-1<     >4294967295<
>%#u<       >2**32-1<     >4294967295<
>%12u<      >2**32-1<     >  4294967295<
>%012u<     >2**32-1<     >004294967295<
>%-12u<     >2**32-1<     >4294967295  <
>%-012u<    >2**32-1<     >4294967295  <
>%4u<       >18<          >  18<
>%4.3u<     >18<          > 018<
>%-4.3u<    >18<          >018 <
>%+4.3u<    >18<          > 018<
>% 4.3u<    >18<          > 018<
>%04.3u<    >18<          > 018<         >0 flag with precision: no effect<
>%.3u<      >18<          >018<
>%v<        >''<          >%v INVALID<
>%w<        >''<          >%w INVALID<
>%x<        >2**32-1<     >ffffffff<
>%+x<       >2**32-1<     >ffffffff<
>%#x<       >2**32-1<     >0xffffffff<
>%10x<      >2**32-1<     >  ffffffff<
>%010x<     >2**32-1<     >00ffffffff<
>%-10x<     >2**32-1<     >ffffffff  <
>%-010x<    >2**32-1<     >ffffffff  <
>%0-10x<    >2**32-1<     >ffffffff  <
>%4x<       >18<          >  12<
>%4.3x<     >18<          > 012<
>%-4.3x<    >18<          >012 <
>%+4.3x<    >18<          > 012<
>% 4.3x<    >18<          > 012<
>%04.3x<    >18<          > 012<         >0 flag with precision: no effect<
>%.3x<      >18<          >012<
>%4X<       >28<          >  1C<
>%4.3X<     >28<          > 01C<
>%-4.3X<    >28<          >01C <
>%+4.3X<    >28<          > 01C<
>% 4.3X<    >28<          > 01C<
>%04.3X<    >28<          > 01C<         >0 flag with precision: no effect<
>%.3X<      >28<          >01C<
>%.0x<      >0<           ><
>%+.0x<     >0<           ><
>% .0x<     >0<           ><
>%-.0x<     >0<           ><
>%#.0x<     >0<           ><
>%#3.0x<    >0<           >   <
>%#3.1x<    >0<           >  0<
>%#3.2x<    >0<           > 00<
>%#3.3x<    >0<           >000<
>%#3.4x<    >0<           >0000<
>%.0x<      >1<           >1<
>%+.0x<     >1<           >1<
>% .0x<     >1<           >1<
>%-.0x<     >1<           >1<
>%#.0x<     >1<           >0x1<
>%#3.0x<    >1<           >0x1<
>%#3.1x<    >1<           >0x1<
>%#3.2x<    >1<           >0x01<
>%#3.3x<    >1<           >0x001<
>%#3.4x<    >1<           >0x0001<
>%#.5x<     >0x12345<     >0x12345<
>%#.5x<     >0x12<        >0x00012<
>%#4x<      >28<          >0x1c<
>%#4.3x<    >28<          >0x01c<
>%#-4.3x<   >28<          >0x01c<
>%#+4.3x<   >28<          >0x01c<
>%# 4.3x<   >28<          >0x01c<
>%#04.3x<   >28<          >0x01c<         >0 flag with precision: no effect<
>%#.3x<     >28<          >0x01c<
>%#6.3x<    >28<          > 0x01c<
>%#-6.3x<   >28<          >0x01c <
>%-#6.3x<   >28<          >0x01c <
>%#+6.3x<   >28<          > 0x01c<
>%+#6.3x<   >28<          > 0x01c<
>%# 6.3x<   >28<          > 0x01c<
>% #6.3x<   >28<          > 0x01c<
>%0*x<      >[-10, ,2**32-1]< >ffffffff  <
>%vx<       >[version::qv("1.2.3")]< >1.2.3<
>%vx<       >[version::qv("1.20.300")]< >1.14.12c<
>%.*x<      >[0,0]<       ><
>%-.*x<     >[0,0]<       ><
>%+.*x<     >[0,0]<       ><
>% .*x<     >[0,0]<       ><
>%0.*x<     >[0,0]<       ><
>%.*x<      >[-3,0]<      >0<
>%-.*x<     >[-3,0]<      >0<
>%+.*x<     >[-3,0]<      >0<
>% .*x<     >[-3,0]<      >0<
>%0.*x<     >[-3,0]<      >0<
>%#.*x<     >[0,0]<       ><
>%#-.*x<    >[0,0]<       ><
>%#+.*x<    >[0,0]<       ><
>%# .*x<    >[0,0]<       ><
>%#0.*x<    >[0,0]<       ><
>%#.*x<     >[-1,0]<      >0<
>%#-.*x<    >[-1,0]<      >0<
>%#+.*x<    >[-1,0]<      >0<
>%# .*x<    >[-1,0]<      >0<
>%#0.*x<    >[-1,0]<      >0<
>%y<        >''<          >%y INVALID<
>%z<        >''<          >%z INVALID<
>%2$d %1$d<	>[12, 34]<	>34 12<
>%*2$d<		>[12, 3]<	> 12<             >RT#125469<
>%*3$d<		>[12, 9, 3]<	> 12<             >related to RT#125469<
>%2$d %d<	>[12, 34]<	>34 12<
>%2$d %d %d<	>[12, 34]<	>34 12 34<
>%3$d %d %d<	>[12, 34, 56]<	>56 12 34<
>%2$*3$d %d<	>[12, 34, 3]<	> 34 12<
>%*3$2$d %d<	>[12, 34, 3]<	>%*3$2$d 12 INVALID<
>%2$d<		>12<	>0 MISSING<
>%0$d<		>12<	>%0$d INVALID<
>%1$$d<		>12<	>%1$$d INVALID<
>%1$1$d<	>12<	>%1$1$d INVALID<
>%*2$*2$d<	>[12, 3]<	>%*2$*2$d INVALID<
>%*2*2$d<	>[12, 3]<	>%*2*2$d INVALID<
>%*2$1d<	>[12, 3]<	>%*2$1d INVALID<
>%0v2.2d<	>''<	><
>%vc,%d<	>[63, 64, 65]<	>%vc,63 INVALID<
>%v%,%d<	>[63, 64, 65]<	>%v%,63 INVALID INVALID<
>%vd,%d<	>["\x1", 2, 3]<	>1,2 REDUNDANT<
>%vf,%d<	>[1, 2, 3]<	>%vf,1 INVALID<
>%vF,%d<	>[1, 2, 3]<	>%vF,1 INVALID<
>%ve,%d<	>[1, 2, 3]<	>%ve,1 INVALID<
>%vE,%d<	>[1, 2, 3]<	>%vE,1 INVALID<
>%vg,%d<	>[1, 2, 3]<	>%vg,1 INVALID<
>%vG,%d<	>[1, 2, 3]<	>%vG,1 INVALID<
>%vp<	>''<	>%vp INVALID<
>%vn<	>''<	>%vn INVALID<
>%vs,%d<	>[1, 2, 3]<	>%vs,1 INVALID<
>%v_<	>''<	>%v_ INVALID<
>%v#x<	>''<	>%v#x INVALID<
>%v02x<	>"\x66\x6f\x6f\012"<	>66.6f.6f.0a<
>%#v.8b<	>"\141\000\142"<	>0b01100001.00000000.0b01100010<	>perl #39530<
>%#v.0o<	>"\001\000\002\000"<    >01.0.02.0<
>%#v.1o<	>"\001\000\002\000"<    >01.0.02.0<
>%#v.4o<	>"\141\000\142"<	>0141.0000.0142<	>perl #39530<
>%#v.3i<	>"\141\000\142"<	>097.000.098<	>perl #39530<
>%#v.0x<	>"\001\000\002\000"<    >0x1..0x2.<
>%#v.1x<	>"\001\000\002\000"<    >0x1.0.0x2.0<
>%#v.2x<	>"\141\000\142"<	>0x61.00.0x62<	>perl #39530<
>%#v.2X<	>"\141\000\142"<	>0X61.00.0X62<	>perl #39530<
>%#v.8b<	>"\141\017\142"<	>0b01100001.0b00001111.0b01100010<	>perl #39530<
>%#v.4o<	>"\141\017\142"<	>0141.0017.0142<	>perl #39530<
>%#v.3i<	>"\141\017\142"<	>097.015.098<	>perl #39530<
>%#v.2x<	>"\141\017\142"<	>0x61.0x0f.0x62<	>perl #39530<
>%#v.2X<	>"\141\017\142"<	>0X61.0X0F.0X62<	>perl #39530<
>%#*v.8b<	>["][", "\141\000\142"]<	>0b01100001][00000000][0b01100010<	>perl #39530<
>%#*v.4o<	>["][", "\141\000\142"]<	>0141][0000][0142<	>perl #39530<
>%#*v.3i<	>["][", "\141\000\142"]<	>097][000][098<	>perl #39530<
>%#*v.2x<	>["][", "\141\000\142"]<	>0x61][00][0x62<	>perl #39530<
>%#*v.2X<	>["][", "\141\000\142"]<	>0X61][00][0X62<	>perl #39530<
>%#*v.8b<	>["][", "\141\017\142"]<	>0b01100001][0b00001111][0b01100010<	>perl #39530<
>%#*v.4o<	>["][", "\141\017\142"]<	>0141][0017][0142<	>perl #39530<
>%#*v.3i<	>["][", "\141\017\142"]<	>097][015][098<	>perl #39530<
>%#*v.2x<	>["][", "\141\017\142"]<	>0x61][0x0f][0x62<	>perl #39530<
>%#*v.2X<	>["][", "\141\017\142"]<	>0X61][0X0F][0X62<	>perl #39530<
>%#v.8b<	>"\141\x{1e01}\000\142\x{1e03}"<	>0b01100001.0b1111000000001.00000000.0b01100010.0b1111000000011<	>perl #39530<
>%#v.4o<	>"\141\x{1e01}\000\142\x{1e03}"<	>0141.017001.0000.0142.017003<	>perl #39530<
>%#v.3i<	>"\141\x{1e01}\000\142\x{1e03}"<	>097.7681.000.098.7683<	>perl #39530<
>%#v.2x<	>"\141\x{1e01}\000\142\x{1e03}"<	>0x61.0x1e01.00.0x62.0x1e03<	>perl #39530<
>%#v.2X<	>"\141\x{1e01}\000\142\x{1e03}"<	>0X61.0X1E01.00.0X62.0X1E03<	>perl #39530<
>%#v.8b<	>"\141\x{1e01}\017\142\x{1e03}"<	>0b01100001.0b1111000000001.0b00001111.0b01100010.0b1111000000011<	>perl #39530<
>%#v.4o<	>"\141\x{1e01}\017\142\x{1e03}"<	>0141.017001.0017.0142.017003<	>perl #39530<
>%#v.3i<	>"\141\x{1e01}\017\142\x{1e03}"<	>097.7681.015.098.7683<	>perl #39530<
>%#v.2x<	>"\141\x{1e01}\017\142\x{1e03}"<	>0x61.0x1e01.0x0f.0x62.0x1e03<	>perl #39530<
>%#v.2X<	>"\141\x{1e01}\017\142\x{1e03}"<	>0X61.0X1E01.0X0F.0X62.0X1E03<	>perl #39530<
>%V-%s<		>["Hello"]<	>%V-Hello INVALID<
>%K %d %d<	>[13, 29]<	>%K 13 29 INVALID<
>%*.*K %d<	>[13, 29, 76]<	>%*.*K 13 INVALID<
>%4$K %d<	>[45, 67]<	>%4$K 45 INVALID<
>%d %K %d<	>[23, 45]<	>23 %K 45 INVALID<
>%*v*999\$d %d %d<	>[11, 22, 33]<	>%*v*999\$d 11 22 INVALID<
>%#b<		>0<	>0<
>%#o<		>0<	>0<
>%#x<		>0<	>0<
>%1073741819$v2d<	>''<	> MISSING<
>%*1073741819$v2d<	>''<	> MISSING<
>%.3X<		>[11]<			>00B<		>perl #83194: hex, zero-padded to 3 places<
>%.*X<		>[3, 11]<		>00B<		>perl #83194: dynamic precision<
a>%vX<		>['012']<		>30.31.32<	>perl #83194: vector flag<
e>%vX<		>['012']<		>F0.F1.F2<	>perl #83194: vector flag<
a>%*vX<		>[':', '012']<		>30:31:32<	>perl #83194: vector flag + custom separator<
e>%*vX<		>[':', '012']<		>F0:F1:F2<	>perl #83194: vector flag + custom separator<
a>%v.3X<		>['012']<		>030.031.032<	>perl #83194: vector flag + static precision<
e>%v.3X<		>['012']<		>0F0.0F1.0F2<	>perl #83194: vector flag + static precision<
a>%v.*X<		>[3, '012']<		>030.031.032<	>perl #83194: vector flag + dynamic precision<
e>%v.*X<		>[3, '012']<		>0F0.0F1.0F2<	>perl #83194: vector flag + dynamic precision<
a>%*v.3X<	>[':', '012']<		>030:031:032<	>perl #83194: vector flag + custom separator + static precision<
e>%*v.3X<	>[':', '012']<		>0F0:0F1:0F2<	>perl #83194: vector flag + custom separator + static precision<
a>%*v.*X<	>[':', 3, '012']<	>030:031:032<	>perl #83194: vector flag + custom separator + dynamic precision<
e>%*v.*X<	>[':', 3, '012']<	>0F0:0F1:0F2<	>perl #83194: vector flag + custom separator + dynamic precision<
a>%vd<	>"version"<	>118.101.114.115.105.111.110<	>perl #102586: vector flag + "version"<
e>%vd<   >"version"<    >165.133.153.162.137.150.149<   >perl #102586: vector flag + "version"<
>%3$*4$v*2$.*1$x<  >[3, 4, "\x11\x22\x33", "/"]< > 011/ 022/ 033< >four reordered args<
>%*%<	>[]<	>% MISSING<
>%*1$%<	>[]<	>% MISSING<
>%*2$d<	>123<	>123 MISSING<
>%2$vd<>123<	> MISSING<
>%.f<   >123.432<   >123<   >by tradition, empty precision == 0 <
>%.001f<   >123.432<   >123.4<   >by tradition, leading zeroes ignored in precison<
>%.0f<   >[1.2, 3.4]<   >1 REDUNDANT<   >special-cased "%.0f" should check count<
>%.0f<   >[]<   >0 MISSING<   >special-cased "%.0f" should check count<
>%53.0f<   >69.0<   >                                                   69<   >#131659<
