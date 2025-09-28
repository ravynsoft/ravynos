use strict;
use warnings;
use ExtUtils::MakeMaker;
use File::Spec;
use Config;
#   [ src => @deps ]
our @files = (
    # compiler files ########################################
    ['regcomp.c'        => 'dquote.c', 'invlist_inline.h'   ],
    ['regcomp_invlist.c' => 'invlist_inline.h'              ],
    ['regcomp_study.c'                                      ],
    ['regcomp_trie.c'                                       ],
    ['regcomp_debug.c'                                      ],
    # execution engine files ################################
    ['regexec.c'        => 'invlist_inline.h'               ],
    # misc files ############################################
    ['dquote.c'                                             ],
    ['invlist_inline.h'                                     ],
    #########################################################
);

my @objects = 're$(OBJ_EXT)';
foreach my $tuple (@files) {
    my $src_file = $tuple->[0];
    if ($src_file=~s/reg/re_/ and $src_file=~s/\.c/\$(OBJ_EXT)/) {
        push @objects, $src_file;
    }
}

my $defines = '-DPERL_EXT_RE_BUILD -DPERL_EXT_RE_DEBUG -DPERL_EXT';

my @libs;
if ($^O eq 'cygwin' && $Config{usequadmath}) {
    push @libs, qw(quadmath);
}
WriteMakefile(
    NAME		=> 're',
    @libs ? ( 'LIBS' => [ join(" ", map { "-l$_" } @libs) ] ) : (),
    VERSION_FROM	=> 're.pm',
    XSPROTOARG		=> '-noprototypes',
    OBJECT		=> "@objects",
    DEFINE             => $defines,
    clean		=> { FILES => '*$(OBJ_EXT) invlist_inline.h *.c ../../lib/re.pm' },
);

package MY;
sub upupfile {
    File::Spec->catfile(File::Spec->updir, File::Spec->updir, $_[0]);
}

sub postamble {
    my $postamble = "";

    foreach my $tuple (@::files) {
        my ($file, @deps) = @$tuple;
        my $src_file = upupfile($file);
        my $target = $file;
        $target =~ s/^reg/re_/;
        $postamble .=
    <<EOF;
$target : $src_file
	- \$(RM_F) $target
	\$(CP) $src_file $target

EOF
        next if $target eq $file;
        my $base_name = $target;
        if ($base_name=~s/\.c\z//) {
            $postamble .= <<EOF
$base_name\$(OBJ_EXT) : $target @deps

EOF
        }
    }
    return $postamble
}

sub MY::c_o {
    my($self) = @_;
    package MY; # so that "SUPER" works right
    my $inh = $self->SUPER::c_o(@_);
    use Config;
    if ($Config{osname} eq 'aix' && $Config{ccversion} eq '5.0.1.0') {
	# Known buggy optimizer.
	my $cccmd = $self->const_cccmd;
	$cccmd =~ s/^CCCMD\s*=\s*//;
	$cccmd =~ s/\s\$\(OPTIMIZE\)\s/ /;
	$inh .= qq{

re_comp\$\(OBJ_EXT\): re_comp.c
\t$cccmd \$(CCCDLFLAGS) -I\$(PERL_INC) \$(DEFINE) \$*.c
};
    }
    $inh;
}
