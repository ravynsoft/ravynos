use SelfLoader;
print "1..1\n";

# this script checks that errors on self-loaded
# subroutines that affect $@ are reported

eval { buggy(); };
unless ($@ =~ /^syntax error/) {
    print "not ";
}
print "ok 1 - syntax errors are reported\n";

__END__

sub buggy
{
    +>*;
}


# RT 40216
#
# by Bo Lindbergh <blgl@hagernas.com>, at Aug 22, 2006 5:42 PM
#
# In the example below, there's a syntax error in the selfloaded
# code for main::buggy.  When the eval fails, SelfLoader::AUTOLOAD
# tries to report this with "croak $@;".  Unfortunately,
# SelfLoader::croak does "require Carp;" without protecting $@,
# which gets clobbered.  The program then dies with the
# uninformative message " at ./example line 3".
#
# #! /usr/local/bin/perl
# use SelfLoader;
# buggy();
# __END__
# sub buggy
# {
#     +>*;
# }
