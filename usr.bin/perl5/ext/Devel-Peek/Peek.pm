# Devel::Peek - A data debugging tool for the XS programmer
# The documentation is after the __END__

package Devel::Peek;

$VERSION = '1.33';
$XS_VERSION = $VERSION;
$VERSION = eval $VERSION;

require Exporter;
require XSLoader;

@EXPORT = qw(Dump mstat DeadCode DumpArray DumpWithOP DumpProg
	     fill_mstats mstats_fillhash mstats2hash runops_debug debug_flags);
@EXPORT_OK = qw(SvREFCNT CvGV);
%EXPORT_TAGS = ('ALL' => [@EXPORT, @EXPORT_OK]);

XSLoader::load();

sub import {
  my $c = shift;
  my $ops_rx = qr/^:opd(=[stP]*)?\b/;
  my @db = grep m/$ops_rx/, @_;
  @_ = grep !m/$ops_rx/, @_;
  if (@db) {
    die "Too many :opd options" if @db > 1;
    runops_debug(1);
    my $flags = ($db[0] =~ m/$ops_rx/ and $1);
    $flags = 'st' unless defined $flags;
    my $f = 0;
    $f |= 2  if $flags =~ /s/;
    $f |= 8  if $flags =~ /t/;
    $f |= 64 if $flags =~ /P/;
    $^D |= $f if $f;
  }
  unshift @_, $c;
  goto &Exporter::import;
}

sub DumpWithOP ($;$) {
   local($Devel::Peek::dump_ops)=1;
   my $depth = @_ > 1 ? $_[1] : 4 ;
   Dump($_[0],$depth);
}

$D_flags = 'psltocPmfrxuLHXDSTR';

sub debug_flags (;$) {
  my $out = "";
  for my $i (0 .. length($D_flags)-1) {
    $out .= substr $D_flags, $i, 1 if $^D & (1<<$i);
  }
  my $arg = shift;
  my $num = $arg;
  if (defined $arg and $arg =~ /\D/) {
    die "unknown flags in debug_flags()" if $arg =~ /[^-$D_flags]/;
    my ($on,$off) = split /-/, "$arg-";
    $num = $^D;
    $num |=  (1<<index($D_flags, $_)) for split //, $on;
    $num &= ~(1<<index($D_flags, $_)) for split //, $off;
  }
  $^D = $num if defined $arg;
  $out
}

sub B::Deparse::pp_Devel_Peek_Dump {
  my ($deparse,$op,$cx) = @_;
  my @kids = $deparse->deparse($op->first, 6);
  my $sib = $op->first->sibling;
  if (ref $sib ne 'B::NULL') {
    push @kids, $deparse->deparse($sib, 6);
  }
  return "Devel::Peek::Dump(" . join(", ", @kids) . ")";
}

1;
__END__

=head1 NAME

Devel::Peek - A data debugging tool for the XS programmer

=head1 SYNOPSIS

        use Devel::Peek;
        Dump( $a );
        Dump( $a, 5 );
        Dump( @a );
        Dump( %h );
        DumpArray( 5, $a, $b, ... );
	mstat "Point 5";

        use Devel::Peek ':opd=st';

=head1 DESCRIPTION

Devel::Peek contains functions which allows raw Perl datatypes to be
manipulated from a Perl script.  This is used by those who do XS programming
to check that the data they are sending from C to Perl looks as they think
it should look.  The trick, then, is to know what the raw datatype is
supposed to look like when it gets to Perl.  This document offers some tips
and hints to describe good and bad raw data.

It is very possible that this document will fall far short of being useful
to the casual reader.  The reader is expected to understand the material in
the first few sections of L<perlguts>.

Devel::Peek supplies a C<Dump()> function which can dump a raw Perl
datatype, and C<mstat("marker")> function to report on memory usage
(if perl is compiled with corresponding option).  The function
DeadCode() provides statistics on the data "frozen" into inactive
C<CV>.  Devel::Peek also supplies C<SvREFCNT()> which can query reference
counts on SVs.  This document will take a passive, and safe, approach
to data debugging and for that it will describe only the C<Dump()>
function.

All output is to STDERR.

The C<Dump()> function takes one or two arguments: something to dump, and
an optional limit for recursion and array elements (default is 4).  The
first argument is evaluated in rvalue scalar context, with exceptions for
@array and %hash, which dump the array or hash itself.  So C<Dump @array>
works, as does C<Dump $foo>.  And C<Dump pos> will call C<pos> in rvalue
context, whereas C<Dump ${\pos}> will call it in lvalue context.

Function C<DumpArray()> allows dumping of multiple values (useful when you
need to analyze returns of functions).

The global variable $Devel::Peek::pv_limit can be set to limit the
number of character printed in various string values.  Setting it to 0
means no limit.

If C<use Devel::Peek> directive has a C<:opd=FLAGS> argument,
this switches on debugging of opcode dispatch.  C<FLAGS> should be a
combination of C<s>, C<t>, and C<P> (see
L<< B<-D> flags in perlrun|perlrun/B<-D>I<letters> >>).

C<:opd> is a shortcut for C<:opd=st>.

=head2 Runtime debugging

C<CvGV($cv)> return one of the globs associated to a subroutine reference $cv.

debug_flags() returns a string representation of C<$^D> (similar to
what is allowed for B<-D> flag).  When called with a numeric argument,
sets $^D to the corresponding value.  When called with an argument of
the form C<"flags-flags">, set on/off bits of C<$^D> corresponding to
letters before/after C<->.  (The returned value is for C<$^D> before
the modification.)

runops_debug() returns true if the current I<opcode dispatcher> is the
debugging one.  When called with an argument, switches to debugging or
non-debugging dispatcher depending on the argument (active for
newly-entered subs/etc only).  (The returned value is for the dispatcher before the modification.)

=head2 Memory footprint debugging

When perl is compiled with support for memory footprint debugging
(default with Perl's malloc()), Devel::Peek provides an access to this API.

Use mstat() function to emit a memory state statistic to the terminal.
For more information on the format of output of mstat() see
L<perldebguts/Using $ENV{PERL_DEBUG_MSTATS}>.

Three additional functions allow access to this statistic from Perl.
First, use C<mstats_fillhash(%hash)> to get the information contained
in the output of mstat() into %hash. The field of this hash are

  minbucket nbuckets sbrk_good sbrk_slack sbrked_remains sbrks
  start_slack topbucket topbucket_ev topbucket_odd total total_chain
  total_sbrk totfree

Two additional fields C<free>, C<used> contain array references which
provide per-bucket count of free and used chunks.  Two other fields
C<mem_size>, C<available_size> contain array references which provide
the information about the allocated size and usable size of chunks in
each bucket.  Again, see L<perldebguts/Using $ENV{PERL_DEBUG_MSTATS}>
for details.


Keep in mind that only the first several "odd-numbered" buckets are
used, so the information on size of the "odd-numbered" buckets which are
not used is probably meaningless.

The information in

 mem_size available_size minbucket nbuckets

is the property of a particular build of perl, and does not depend on
the current process.  If you do not provide the optional argument to
the functions mstats_fillhash(), fill_mstats(), mstats2hash(), then
the information in fields C<mem_size>, C<available_size> is not
updated.

C<fill_mstats($buf)> is a much cheaper call (both speedwise and
memory-wise) which collects the statistic into $buf in
machine-readable form.  At a later moment you may need to call
C<mstats2hash($buf, %hash)> to use this information to fill %hash.

All three APIs C<fill_mstats($buf)>, C<mstats_fillhash(%hash)>, and
C<mstats2hash($buf, %hash)> are designed to allocate no memory if used
I<the second time> on the same $buf and/or %hash.

So, if you want to collect memory info in a cycle, you may call

  $#buf = 999;
  fill_mstats($_) for @buf;
  mstats_fillhash(%report, 1);		# Static info too

  foreach (@buf) {
    # Do something...
    fill_mstats $_;			# Collect statistic
  }
  foreach (@buf) {
    mstats2hash($_, %report);		# Preserve static info
    # Do something with %report
  }

=head1 EXAMPLES

The following examples don't attempt to show everything as that would be a
monumental task, and, frankly, we don't want this manpage to be an internals
document for Perl.  The examples do demonstrate some basics of the raw Perl
datatypes, and should suffice to get most determined people on their way.
There are no guidewires or safety nets, nor blazed trails, so be prepared to
travel alone from this point and on and, if at all possible, don't fall into
the quicksand (it's bad for business).

Oh, one final bit of advice: take L<perlguts> with you.  When you return we
expect to see it well-thumbed.

=head2 A simple scalar string

Let's begin by looking a simple scalar which is holding a string.

        use Devel::Peek;
        $a = 42; $a = "hello";
        Dump $a;

The output:

        SV = PVIV(0xbc288) at 0xbe9a8
          REFCNT = 1
          FLAGS = (POK,pPOK)
          IV = 42
          PV = 0xb2048 "hello"\0
          CUR = 5
          LEN = 8

This says C<$a> is an SV, a scalar.  The scalar type is a PVIV, which is
capable of holding an integer (IV) and/or a string (PV) value. The scalar's
head is allocated at address 0xbe9a8, while the body is at 0xbc288.
Its reference count is 1.  It has the C<POK> flag set, meaning its
current PV field is valid.  Because POK is set we look at the PV item
to see what is in the scalar.  The \0 at the end indicate that this
PV is properly NUL-terminated.
Note that the IV field still contains its old numeric value, but because
FLAGS doesn't have IOK set, we must ignore the IV item.
CUR indicates the number of characters in the PV.  LEN indicates the
number of bytes allocated for the PV (at least one more than CUR, because
LEN includes an extra byte for the end-of-string marker, then usually
rounded up to some efficient allocation unit).

=head2 A simple scalar number

If the scalar contains a number the raw SV will be leaner.

        use Devel::Peek;
        $a = 42;
        Dump $a;

The output:

        SV = IV(0xbc818) at 0xbe9a8
          REFCNT = 1
          FLAGS = (IOK,pIOK)
          IV = 42

This says C<$a> is an SV, a scalar.  The scalar is an IV, a number.  Its
reference count is 1.  It has the C<IOK> flag set, meaning it is currently
being evaluated as a number.  Because IOK is set we look at the IV item to
see what is in the scalar.

=head2 A simple scalar with an extra reference

If the scalar from the previous example had an extra reference:

        use Devel::Peek;
        $a = 42;
        $b = \$a;
        Dump $a;

The output:

        SV = IV(0xbe860) at 0xbe9a8
          REFCNT = 2
          FLAGS = (IOK,pIOK)
          IV = 42

Notice that this example differs from the previous example only in its
reference count.  Compare this to the next example, where we dump C<$b>
instead of C<$a>.

=head2 A reference to a simple scalar

This shows what a reference looks like when it references a simple scalar.

        use Devel::Peek;
        $a = 42;
        $b = \$a;
        Dump $b;

The output:

        SV = IV(0xf041c) at 0xbe9a0
          REFCNT = 1
          FLAGS = (ROK)
          RV = 0xbab08
          SV = IV(0xbe860) at 0xbe9a8
            REFCNT = 2
            FLAGS = (IOK,pIOK)
            IV = 42

Starting from the top, this says C<$b> is an SV.  The scalar is an IV,
which is capable of holding an integer or reference value.
It has the C<ROK> flag set, meaning it is a reference (rather than an
integer or string).  Notice that Dump
follows the reference and shows us what C<$b> was referencing.  We see the
same C<$a> that we found in the previous example.

Note that the value of C<RV> coincides with the numbers we see when we
stringify $b. The addresses inside IV() are addresses of
C<X***> structures which hold the current state of an C<SV>. This
address may change during lifetime of an SV.

=head2 A reference to an array

This shows what a reference to an array looks like.

        use Devel::Peek;
        $a = [42];
        Dump $a;

The output:

        SV = IV(0xc85998) at 0xc859a8
          REFCNT = 1
          FLAGS = (ROK)
          RV = 0xc70de8
          SV = PVAV(0xc71e10) at 0xc70de8
            REFCNT = 1
            FLAGS = ()
            ARRAY = 0xc7e820
            FILL = 0
            MAX = 0
            FLAGS = (REAL)
            Elt No. 0
            SV = IV(0xc70f88) at 0xc70f98
              REFCNT = 1
              FLAGS = (IOK,pIOK)
              IV = 42

This says C<$a> is a reference (ROK), which points to
another SV which is a PVAV, an array.  The array has one element,
element zero, which is another SV. The field C<FILL> above indicates
the last element in the array, similar to C<$#$a>.

If C<$a> pointed to an array of two elements then we would see the
following.

        use Devel::Peek 'Dump';
        $a = [42,24];
        Dump $a;

The output:

        SV = IV(0x158c998) at 0x158c9a8
          REFCNT = 1
          FLAGS = (ROK)
          RV = 0x1577de8
          SV = PVAV(0x1578e10) at 0x1577de8
            REFCNT = 1
            FLAGS = ()
            ARRAY = 0x1585820
            FILL = 1
            MAX = 1
            FLAGS = (REAL)
            Elt No. 0
            SV = IV(0x1577f88) at 0x1577f98
              REFCNT = 1
              FLAGS = (IOK,pIOK)
              IV = 42
            Elt No. 1
            SV = IV(0x158be88) at 0x158be98
              REFCNT = 1
              FLAGS = (IOK,pIOK)
              IV = 24

Note that C<Dump> will not report I<all> the elements in the array,
only several first (depending on how deep it already went into the
report tree).

=head2 A reference to a hash

The following shows the raw form of a reference to a hash.

        use Devel::Peek;
        $a = {hello=>42};
        Dump $a;

The output:

    SV = IV(0x55cb50b50fb0) at 0x55cb50b50fc0
      REFCNT = 1
      FLAGS = (ROK)
      RV = 0x55cb50b2b758
      SV = PVHV(0x55cb50b319c0) at 0x55cb50b2b758
        REFCNT = 1
        FLAGS = (SHAREKEYS)
        ARRAY = 0x55cb50b941a0  (0:7, 1:1)
        hash quality = 100.0%
        KEYS = 1
        FILL = 1
        MAX = 7
        Elt "hello" HASH = 0x3128ece4
        SV = IV(0x55cb50b464f8) at 0x55cb50b46508
          REFCNT = 1
          FLAGS = (IOK,pIOK)
          IV = 42

This shows C<$a> is a reference pointing to an SV.  That SV is a PVHV, a hash.

The "quality" of a hash is defined as the total number of comparisons needed
to access every element once, relative to the expected number needed for a
random hash. The value can go over 100%.

The total number of comparisons is equal to the sum of the squares of the
number of entries in each bucket.  For a random hash of C<<n>> keys into
C<<k>> buckets, the expected value is:

		n + n(n-1)/2k

=head2 Dumping a large array or hash

The C<Dump()> function, by default, dumps up to 4 elements from a
toplevel array or hash.  This number can be increased by supplying a
second argument to the function.

        use Devel::Peek;
        $a = [10,11,12,13,14];
        Dump $a;

Notice that C<Dump()> prints only elements 10 through 13 in the above code.
The following code will print all of the elements.

        use Devel::Peek 'Dump';
        $a = [10,11,12,13,14];
        Dump $a, 5;

=head2 A reference to an SV which holds a C pointer

This is what you really need to know as an XS programmer, of course.  When
an XSUB returns a pointer to a C structure that pointer is stored in an SV
and a reference to that SV is placed on the XSUB stack.  So the output from
an XSUB which uses something like the T_PTROBJ map might look something like
this:

        SV = IV(0xf381c) at 0xc859a8
          REFCNT = 1
          FLAGS = (ROK)
          RV = 0xb8ad8
          SV = PVMG(0xbb3c8) at 0xc859a0
            REFCNT = 1
            FLAGS = (OBJECT,IOK,pIOK)
            IV = 729160
            NV = 0
            PV = 0
            STASH = 0xc1d10       "CookBookB::Opaque"

This shows that we have an SV which is a reference, which points at another
SV.  In this case that second SV is a PVMG, a blessed scalar.  Because it is
blessed it has the C<OBJECT> flag set.  Note that an SV which holds a C
pointer also has the C<IOK> flag set.  The C<STASH> is set to the package
name which this SV was blessed into.

The output from an XSUB which uses something like the T_PTRREF map, which
doesn't bless the object, might look something like this:

        SV = IV(0xf381c) at 0xc859a8
          REFCNT = 1
          FLAGS = (ROK)
          RV = 0xb8ad8
          SV = PVMG(0xbb3c8) at 0xc859a0
            REFCNT = 1
            FLAGS = (IOK,pIOK)
            IV = 729160
            NV = 0
            PV = 0

=head2 A reference to a subroutine

Looks like this:

	SV = IV(0x24d2dd8) at 0x24d2de8
	  REFCNT = 1
	  FLAGS = (TEMP,ROK)
	  RV = 0x24e79d8
	  SV = PVCV(0x24e5798) at 0x24e79d8
	    REFCNT = 2
	    FLAGS = ()
	    COMP_STASH = 0x22c9c50	"main"
	    START = 0x22eed60 ===> 0
	    ROOT = 0x22ee490
	    GVGV::GV = 0x22de9d8	"MY" :: "top_targets"
	    FILE = "(eval 5)"
	    DEPTH = 0
	    FLAGS = 0x0
	    OUTSIDE_SEQ = 93
	    PADLIST = 0x22e9ed8
	    PADNAME = 0x22e9ec0(0x22eed00) PAD = 0x22e9ea8(0x22eecd0)
	    OUTSIDE = 0x22c9fb0 (MAIN)


This shows that 

=over 4

=item *

the subroutine is not an XSUB (since C<START> and C<ROOT> are
non-zero, and C<XSUB> is not listed, and is thus null);

=item *

that it was compiled in the package C<main>;

=item *

under the name C<MY::top_targets>; 

=item *

inside a 5th eval in the program;

=item *

it is not currently executed (because C<DEPTH> is 0);

=item *

it has no prototype (C<PROTOTYPE> field is missing).

=back

=head1 EXPORTS

C<Dump>, C<mstat>, C<DeadCode>, C<DumpArray>, C<DumpWithOP> and
C<DumpProg>, C<fill_mstats>, C<mstats_fillhash>, C<mstats2hash> by
default. Additionally available C<SvREFCNT>, C<SvREFCNT_inc> and
C<SvREFCNT_dec>.

=head1 BUGS

Readers have been known to skip important parts of L<perlguts>, causing much
frustration for all.

=head1 AUTHOR

Ilya Zakharevich	ilya@math.ohio-state.edu

Copyright (c) 1995-98 Ilya Zakharevich. All rights reserved.
This program is free software; you can redistribute it and/or
modify it under the same terms as Perl itself.

Author of this software makes no claim whatsoever about suitability,
reliability, edability, editability or usability of this product, and
should not be kept liable for any damage resulting from the use of
it. If you can use it, you are in luck, if not, I should not be kept
responsible. Keep a handy copy of your backup tape at hand.

=head1 SEE ALSO

L<perlguts>, and L<perlguts>, again.

=cut
