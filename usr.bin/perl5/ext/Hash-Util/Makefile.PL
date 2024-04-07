use ExtUtils::MakeMaker;

# this file was templated from ext/List/Util/Makefile.PL
# thanks to Graham Barr who wrote that module.

WriteMakefile(
    VERSION_FROM    => "lib/Hash/Util.pm",
    NAME            => "Hash::Util",
    DEFINE          => "-DPERL_EXT",
);

package MY;

# We go through the HashUtil.c trickery to foil platforms
# that have the feature combination of
# (1) static builds
# (2) allowing only one object by the same name in the static library
# (3) the object name matching being case-blind
# This means that we can't have the top-level util.o
# and the extension-level Util.o in the same build.
# One such platform is the POSIX-BC BS2000 EBCDIC mainframe platform.

BEGIN {
    use Config;
    unless (defined $Config{usedl}) {
	eval <<'__EOMM__';
sub xs_c {
    my($self) = shift;
    return '' unless $self->needs_linking();
'
HashUtil.c:	Util.xs
	$(PERL) -I$(PERL_ARCHLIB) -I$(PERL_LIB) $(XSUBPP) $(XSPROTOARG) $(XSUBPPARGS) Util.xs > HashUtil.xsc && $(MV) HashUtil.xsc HashUtil.c
';
}

sub xs_o {
    my($self) = shift;
    return '' unless $self->needs_linking();
'

Util$(OBJ_EXT):	HashUtil.c
	$(CCCMD) $(CCCDLFLAGS) -I$(PERL_INC) $(DEFINE) HashUtil.c
	$(MV) HashUtil$(OBJ_EXT) Util$(OBJ_EXT)
';
}

__EOMM__
    }
}
