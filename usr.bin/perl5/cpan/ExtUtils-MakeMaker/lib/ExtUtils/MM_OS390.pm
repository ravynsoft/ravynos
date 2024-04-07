package ExtUtils::MM_OS390;

use strict;
use warnings;
our $VERSION = '7.70';
$VERSION =~ tr/_//d;

use ExtUtils::MakeMaker::Config;
require ExtUtils::MM_Unix;
our @ISA = qw(ExtUtils::MM_Unix);

=head1 NAME

ExtUtils::MM_OS390 - OS390 specific subclass of ExtUtils::MM_Unix

=head1 SYNOPSIS

  Don't use this module directly.
  Use ExtUtils::MM and let it choose.

=head1 DESCRIPTION

This is a subclass of L<ExtUtils::MM_Unix> which contains functionality for
OS390.

Unless otherwise stated it works just like ExtUtils::MM_Unix.

=head2 Overriden methods

=over

=item xs_make_dynamic_lib

Defines the recipes for the C<dynamic_lib> section.

=cut

sub xs_make_dynamic_lib {
    my ($self, $attribs, $object, $to, $todir, $ldfrom, $exportlist, $dlsyms) = @_;
    $exportlist = '' if $exportlist ne '$(EXPORT_LIST)';
    my $armaybe = $self->_xs_armaybe($attribs);
    my @m = sprintf '%s : %s $(MYEXTLIB) %s$(DFSEP).exists %s $(PERL_ARCHIVEDEP) $(PERL_ARCHIVE_AFTER) $(INST_DYNAMIC_DEP) %s'."\n", $to, $object, $todir, $exportlist, ($dlsyms || '');
    my $dlsyms_arg = $self->xs_dlsyms_arg($dlsyms);
    if ($armaybe ne ':'){
        $ldfrom = 'tmp$(LIB_EXT)';
        push(@m,"	\$(ARMAYBE) cr $ldfrom $object\n");
        push(@m,"	\$(RANLIB) $ldfrom\n");
    }

    # For example in AIX the shared objects/libraries from previous builds
    # linger quite a while in the shared dynalinker cache even when nobody
    # is using them.  This is painful if one for instance tries to restart
    # a failed build because the link command will fail unnecessarily 'cos
    # the shared object/library is 'busy'.
    push(@m,"	\$(RM_F) \$\@\n");

    my $libs = '$(LDLOADLIBS)';

    my $ld_run_path_shell = "";
    if ($self->{LD_RUN_PATH} ne "") {
        $ld_run_path_shell = 'LD_RUN_PATH="$(LD_RUN_PATH)" ';
    }

    push @m, sprintf <<'MAKE', $ld_run_path_shell, $self->xs_obj_opt('$@'), $dlsyms_arg, $ldfrom, $libs, $exportlist;
	%s$(LD) %s $(LDDLFLAGS) %s $(OTHERLDFLAGS) %s $(MYEXTLIB) \
	  $(PERL_ARCHIVE) %s $(PERL_ARCHIVE_AFTER) %s \
	  $(INST_DYNAMIC_FIX)
	$(CHMOD) $(PERM_RWX) $@
MAKE
    join '', @m;
}

1;

=back

=head1 AUTHOR

Michael G Schwern <schwern@pobox.com> with code from ExtUtils::MM_Unix

=head1 SEE ALSO

L<ExtUtils::MakeMaker>

=cut
__END__
