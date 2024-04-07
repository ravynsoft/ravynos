#!perl
#===============================================================================
#
# Makefile.PL
#
# DESCRIPTION
#   Makefile creation script.
#
# COPYRIGHT
#   Copyright (C) 2014-2015, 2020 Steve Hay.  All rights reserved.
#
# LICENCE
#   This script is free software; you can redistribute it and/or modify it under
#   the same terms as Perl itself, i.e. under the terms of either the GNU
#   General Public License or the Artistic License, as specified in the LICENCE
#   file.
#
#===============================================================================

use 5.008001;

use strict;
use warnings;

use ExtUtils::MakeMaker 6.64;
use ExtUtils::MakeMaker qw(WriteMakefile);

## no critic (Subroutines::ProhibitSubroutinePrototypes)

sub running_under_cpan();

#===============================================================================
# INITIALIZATION
#===============================================================================

our($CfgFile, $CfgPath);

BEGIN {
    $CfgFile = 'libnet.cfg';
    $CfgPath = "Net/$CfgFile";
}

#===============================================================================
# MAIN PROGRAM
#===============================================================================

MAIN: {
    my %prereq_pms = ();
    $prereq_pms{'Convert::EBCDIC'} = '0.06' if $^O eq 'os390';

    my $xt = 'n';
    if (not running_under_cpan() and not $ENV{PERL_CORE}) {
        $xt = prompt("Should I do external tests?\n" .
                     "These tests will fail if there is no internet" .
                     " connection or if a firewall\n" .
                     "blocks or modifies some traffic.\n" .
                     "[y/N]", 'n');
    }

    my $tests = 't/*.t';
    $tests .= ' t/external/*.t' if $xt =~ m/^y/io;

    WriteMakefile(
        NAME     => 'Net',
        DISTNAME => 'libnet',
        ABSTRACT => 'Collection of network protocol modules',
        AUTHOR   => 'Graham Barr <gbarr@pobox.com>, Steve Hay <shay@cpan.org>',
        LICENSE  => 'perl_5',
        VERSION  => '3.15',

        META_MERGE => {
            'meta-spec' => {
                version => 2
            },

            resources => {
                repository => {
                    type => 'git',
                    web  => 'https://github.com/steve-m-hay/perl-libnet'
                }
            },

            optional_features => {
                APOP => {
                    description => 'APOP support',
                    prereqs => {
                        runtime => {
                            requires => {
                                'Digest::MD5' => '0'
                            }
                        }
                    }
                },

                AUTH => {
                    description => 'AUTH support',
                    prereqs => {
                        runtime => {
                            requires => {
                                'Authen::SASL' => '0',
                                'MIME::Base64' => '0'
                            }
                        }
                    }
                },

                SSL => {
                    description => 'SSL support',
                    prereqs => {
                        runtime => {
                            requires => {
                                'IO::Socket::SSL' => '2.007'
                            }
                        }
                    }
                },

                IPv6 => {
                    description => 'IPv6 support',
                    prereqs => {
                        runtime => {
                            requires => {
                                'IO::Socket::IP' => '0.25'
                            # or IO::Socket::INET6 2.62
                            }
                        }
                    }
                },

                changestest => {
                    description => 'Changes testing',
                    prereqs => {
                        test => {
                            requires => {
                                'Test::CPAN::Changes' => '0'
                            }
                        }
                    }
                },

                critictest => {
                    description => 'Perl::Critic testing',
                    prereqs => {
                        test => {
                            requires => {
                                'Test::Perl::Critic' => '0'
                            }
                        }
                    }
                },

                podtest => {
                    description => 'POD testing',
                    prereqs => {
                        test => {
                            requires => {
                                'Test::Pod' => '1.00'
                            }
                        }
                    }
                },

                podcoveragetest => {
                    description => 'POD coverage testing',
                    prereqs => {
                        test => {
                            requires => {
                                'Test::Pod::Coverage' => '0.08'
                            }
                        }
                    }
                }
            }
        },

        MIN_PERL_VERSION => '5.008001',

        CONFIGURE_REQUIRES => {
            'ExtUtils::MakeMaker' => '6.64',
            'Getopt::Std'         => '0',
            'IO::File'            => '0',
            'perl'                => '5.008001',
            'strict'              => '0',
            'vars'                => '0',
            'warnings'            => '0'
        },

        TEST_REQUIRES => {
            'Config' => '0',
            'Cwd'    => '0'
        },

        PREREQ_PM => {
            %prereq_pms,
            'Carp'           => '0',
            'Errno'          => '0',
            'Exporter'       => '0',
            'Fcntl'          => '0',
            'File::Basename' => '0',
            'FileHandle'     => '0',
            'IO::Select'     => '0',
            'IO::Socket'     => '1.05',
            'POSIX'          => '0',
            'Socket'         => '2.016',
            'Symbol'         => '0',
            'Time::Local'    => '0',
            'constant'       => '0',
            'strict'         => '0',
            'utf8'           => '0',
            'vars'           => '0'
        },

        INSTALLDIRS => ($] < 5.011 ? 'perl' : 'site'),

        realclean => {
            FILES => $CfgFile
        },

        test => {
            TESTS => $tests
        },

        dist => {
            PREOP   => 'find $(DISTVNAME) -type d -print|xargs chmod 0755 && ' .
                       'find $(DISTVNAME) -type f -print|xargs chmod 0644',
            TO_UNIX => 'find $(DISTVNAME) -type f -print|xargs dos2unix'
        }
    );
}

#===============================================================================
# MAKEMAKER OVERRIDES
#===============================================================================

sub MY::post_initialize {
    my $self = shift;

    return '' if $self->{PERL_CORE};

    if (not -f $CfgFile) {
        my @args = qw(Configure);
        push @args, '-d' if $ENV{PERL5_CPAN_IS_RUNNING}     ||
                            $ENV{PERL5_CPANPLUS_IS_RUNNING} ||
                            $ENV{PERL5_CPANM_IS_RUNNING};
        system(($^O eq 'VMS' ? 'mcr ': ()), $^X, @args)
    }

    $self->{PM}{$CfgFile} = $self->catfile('$(INST_LIBDIR)',$CfgPath);

    return '';
}

#===============================================================================
# SUBROUTINES
#===============================================================================

sub running_under_cpan() {
    return $ENV{PERL5_CPAN_IS_RUNNING}     || # cpan
           $ENV{PERL5_CPANPLUS_IS_RUNNING} || # cpanp
           $ENV{PERL5_CPANM_IS_RUNNING};      # cpanm
}

#===============================================================================
