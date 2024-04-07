use 5.008;
use ExtUtils::MakeMaker;
use ExtUtils::Constant 0.11 'WriteConstants';
use Config;

my $dtrace_o = $Config{dtraceobject} ? ' dtrace$(OBJ_EXT)' : '';

WriteMakefile(
    'NAME'		=> 'XS::APItest',
    'VERSION_FROM'	=> 'APItest.pm', # finds $VERSION
    'PREREQ_PM'		=> {}, # e.g., Module::Name => 1.1
    ABSTRACT_FROM	=> 'APItest.pm', # retrieve abstract from module
    AUTHOR		=> 'Tim Jenness <t.jenness@jach.hawaii.edu>, Christian Soeller <csoelle@mph.auckland.ac.nz>, Hugo van der Sanden <hv@crypt.compulink.co.uk>, Andrew Main (Zefram) <zefram@fysh.org>',
    'C'                 => ['exception.c', 'core.c', 'notcore.c'],
    'OBJECT'            => '$(BASEEXT)$(OBJ_EXT) XSUB-undef-XS_VERSION$(OBJ_EXT) XSUB-redefined-macros$(OBJ_EXT) $(O_FILES)'. $dtrace_o,
    realclean => {FILES	=> 'const-c.inc const-xs.inc'},
    ($Config{gccversion} && $Config{d_attribute_deprecated} ?
      (CCFLAGS => $Config{ccflags} . ' -Wno-deprecated-declarations') : ()),
    depend => { 'core.o' => 'core_or_not.inc',
		'notcore.o' => 'core_or_not.inc' },
);

my @names = (qw(HV_DELETE HV_DISABLE_UVAR_XKEY HV_FETCH_ISSTORE
		HV_FETCH_ISEXISTS HV_FETCH_LVALUE HV_FETCH_JUST_SV
		G_SCALAR G_LIST G_VOID G_DISCARD G_EVAL G_NOARGS
		G_KEEPERR G_NODEBUG G_METHOD G_FAKINGEVAL G_RETHROW
		GV_NOADD_NOINIT
		SV_GMAGIC SV_SKIP_OVERLOAD
		IS_NUMBER_IN_UV IS_NUMBER_GREATER_THAN_UV_MAX
		IS_NUMBER_NOT_INT IS_NUMBER_NEG IS_NUMBER_INFINITY
		IS_NUMBER_NAN IS_NUMBER_TRAILING PERL_SCAN_TRAILING
		PERL_LOADMOD_DENY PERL_LOADMOD_NOIMPORT PERL_LOADMOD_IMPORT_OPS
		),
	     {name=>"G_WANT", default=>["IV", "G_LIST|G_VOID"]});

open my $fh, '<', '../../overload.h' or die "Can't open ../../overload.h: $!";
while (<$fh>) {
    push @names, {name => $1, macro => 1} if /^\s+([A-Za-z_0-9]+_amg),/;
}

WriteConstants(
    PROXYSUBS => 1,
    NAME => 'XS::APItest',
    NAMES => \@names,
	       );

sub MY::install { "install ::\n"  };


sub MY::postamble
{
    package MY;
    my $post = shift->SUPER::postamble(@_);
    use Config;
    return $post unless $Config{dtraceobject};

    # core.o is build using PERL_CORE, so picks up any dtrace probes

    $post .= <<POSTAMBLE;

DTRACE_D = ../../perldtrace.d

dtrace\$(OBJ_EXT): \$(DTRACE_D) core\$(OBJ_EXT)
	$Config{dtrace} -G -s \$(DTRACE_D) -o dtrace\$(OBJ_EXT) core\$(OBJ_EXT) || \\
	  ( \$(ECHO) >dtrace.c && \$(CCCMD) \$(CCCDLFLAGS) dtrace.c && rm -f dtrace.c )
POSTAMBLE

    return $post;
}
