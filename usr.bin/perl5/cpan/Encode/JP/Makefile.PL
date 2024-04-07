use 5.7.2;
use strict;
use ExtUtils::MakeMaker;
use strict;

my %tables = (
          euc_jp_t   => ['euc-jp.ucm'],
              sjis_t     => ['shiftjis.ucm',
                 'macJapanese.ucm',
                 'cp932.ucm'],
          raw_t      => [
                 qw(jis0201.ucm jis0208.ucm jis0212.ucm)
                 ],
             );

unless ($ENV{AGGREGATE_TABLES}){
    my @ucm;
    for my $k (keys %tables){
    push @ucm, @{$tables{$k}};
    }
    %tables = ();
    my $seq = 0;
    for my $ucm (sort @ucm){
    # 8.3 compliance !
    my $t = sprintf ("%s_%02d_t", substr($ucm, 0, 2), $seq++);
    $tables{$t} = [ $ucm ];
    }
}

my $name = 'JP';

WriteMakefile(
              INC		=> "-I../Encode",
          NAME		=> 'Encode::'.$name,
          VERSION_FROM	=> "$name.pm",
          OBJECT		=> '$(O_FILES)',
          'dist'		=> {
          COMPRESS	=> 'gzip -9f',
          SUFFIX	=> 'gz',
          DIST_DEFAULT => 'all tardist',
          },
          MAN3PODS	=> {},
          # OS 390 winges about line numbers > 64K ???
          XSOPT => '-nolinenumbers',
          );

package MY;

sub post_initialize
{
    my ($self) = @_;
    my %o;
    my $x = $self->{'OBJ_EXT'};
    # Add the table O_FILES
    foreach my $e (keys %tables)
    {
    $o{$e.$x} = 1;
    }
    $o{"$name$x"} = 1;
    $self->{'O_FILES'} = [sort keys %o];
    my @files = ("$name.xs");
    $self->{'C'} = ["$name.c"];
    $self->{SOURCE} .= " $name.c"
        if $^O eq 'MacOS' && $self->{SOURCE} !~ /\b$name\.c\b/;
    $self->{'H'} = [$self->catfile($self->updir,'Encode', 'encode.h')];
    my %xs;
    foreach my $table (sort keys %tables) {
    push (@{$self->{'C'}},"$table.c");
    # Do NOT add $table.h etc. to H_FILES unless we own up as to how they
    # get built.
    foreach my $ext (qw($(OBJ_EXT) .c .h .exh .fnm)) {
        push (@files,$table.$ext);
    }
    $self->{SOURCE} .= " $table.c"
        if $^O eq 'MacOS' && $self->{SOURCE} !~ /\b$table\.c\b/;
    }
    $self->{'XS'} = { "$name.xs" => "$name.c" };
    $self->{'clean'}{'FILES'} .= join(' ',@files);
    open(XS,">$name.xs") || die "Cannot open $name.xs:$!";
    print XS <<'END';
#define PERL_NO_GET_CONTEXT
#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>
#include "encode.h"
END
    foreach my $table (sort keys %tables) {
    print XS qq[#include "${table}.h"\n];
    }
    print XS <<"END";

static void
Encode_XSEncoding(pTHX_ encode_t *enc)
{
 dSP;
 HV *stash = gv_stashpv("Encode::XS", TRUE);
 SV *iv    = newSViv(PTR2IV(enc));
 SV *sv    = sv_bless(newRV_noinc(iv),stash);
 int i = 0;
 /* with the SvLEN() == 0 hack, PVX won't be freed. We cast away name's
 constness, in the hope that perl won't mess with it. */
 assert(SvTYPE(iv) >= SVt_PV); assert(SvLEN(iv) == 0);
 SvFLAGS(iv) |= SVp_POK;
 SvPVX(iv) = (char*) enc->name[0];
 PUSHMARK(sp);
 XPUSHs(sv);
 while (enc->name[i])
  {
   const char *name = enc->name[i++];
   XPUSHs(sv_2mortal(newSVpvn(name,strlen(name))));
  }
 PUTBACK;
 call_pv("Encode::define_encoding",G_DISCARD);
 SvREFCNT_dec(sv);
}

MODULE = Encode::$name	PACKAGE = Encode::$name
PROTOTYPES: DISABLE
BOOT:
{
END
    foreach my $table (sort keys %tables) {
    print XS qq[#include "${table}.exh"\n];
    }
    print XS "}\n";
    close(XS);
    return "# Built $name.xs\n\n";
}

sub postamble
{
    my $self = shift;
    my $dir  = $self->catdir($self->updir,'ucm');
    my $str  = "# $name\$(OBJ_EXT) depends on .h and .exh files not .c files - but all written by enc2xs\n";
    $str    .= "$name.c : $name.xs ";
    foreach my $table (sort keys %tables)
    {
    $str .= " $table.c";
    }
    $str .= "\n\n";
    $str .= "$name\$(OBJ_EXT) : $name.c\n\n";

    my $enc2xs = $self->catfile($self->updir,'bin', 'enc2xs');
    foreach my $table (sort keys %tables)
    {
    my $numlines = 1;
    my $lengthsofar = length($str);
    my $continuator = '';
    $str .= "$table.c : $enc2xs Makefile.PL";
    foreach my $file (@{$tables{$table}})
    {
        $str .= $continuator.' '.$self->catfile($dir,$file);
        if ( length($str)-$lengthsofar > 128*$numlines )
        {
        $continuator .= " \\\n\t";
        $numlines++;
        } else {
        $continuator = '';
        }
    }
    my $plib   = $self->{PERL_CORE} ? '"-I$(PERL_LIB)"' : '';
    $plib .= " -MCross=$::Cross::platform" if defined $::Cross::platform;
    my $ucopts = '-"Q"';
    $str .=  
        qq{\n\t\$(PERL) $plib $enc2xs $ucopts -o \$\@ -f $table.fnm\n\n};
    open (FILELIST, ">$table.fnm")
        || die "Could not open $table.fnm: $!";
    foreach my $file (@{$tables{$table}})
    {
        print FILELIST $self->catfile($dir,$file) . "\n";
    }
    close(FILELIST);
    }
    return $str;
}

