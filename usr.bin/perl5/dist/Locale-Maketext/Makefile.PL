require 5.006; # uses 'our'
use strict;
use ExtUtils::MakeMaker;

WriteMakefile(
    NAME          => 'Locale::Maketext',
    VERSION_FROM  => 'lib/Locale/Maketext.pm',
    ABSTRACT_FROM => 'lib/Locale/Maketext.pod',
    PREREQ_PM     => {
        'I18N::LangTags'         => 0.31,
        'I18N::LangTags::Detect' => 0,
        'Test::More'             => 0,
        'parent'                 => 0,      # For testing t/30_eval_dollar_at.t
    },
    dist => { COMPRESS => 'gzip -9f', SUFFIX => 'gz', },
    ( $ExtUtils::MakeMaker::VERSION >= 6.3002 ? ( 'LICENSE' => 'perl', ) : () ),
    INSTALLDIRS => ( $] < 5.011 ? 'perl' : 'site' ),

    # If under a version with Maketext in core, overwrite that core file.
    META_MERGE => {
        resources => {
            license     => 'http://dev.perl.org/licenses/',
            bugtracker  => 'https://github.com/perl/perl5/issues',
            repository  => 'https://github.com/Perl/perl5/tree/blead/dist/Locale-Maketext',
            MailingList => 'http://lists.perl.org/list/perl5-porters.html',
        },
    },
);

sub MY::postamble {
    # .PHONY is not portable
    my $self = shift;
    my $phony_line = $self->can('is_make_type')
                     && ($self->is_make_type('gmake')
                         || $self->is_make_type('bsdmake'))
                     ? '.PHONY: tags critic'
                     : '';

    return "$phony_line\n\n" . <<'MAKE_FRAG';

tags:
	ctags -f tags --recurse --totals \
		--exclude=blib \
		--exclude=.svn \
		--exclude='*~' \
		--languages=Perl --langmap=Perl:+.t \

critic:
	perlcritic -1 -q -profile perlcriticrc -statistics lib/ t/

MAKE_FRAG
}
