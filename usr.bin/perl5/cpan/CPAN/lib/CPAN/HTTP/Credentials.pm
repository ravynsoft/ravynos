# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
# vim: ts=4 sts=4 sw=4:
package CPAN::HTTP::Credentials;
use strict;
use vars qw($USER $PASSWORD $PROXY_USER $PROXY_PASSWORD);

$CPAN::HTTP::Credentials::VERSION = $CPAN::HTTP::Credentials::VERSION = "1.9601";

sub clear_credentials {
   clear_non_proxy_credentials();
   clear_proxy_credentials();
}

sub clear_non_proxy_credentials {
    undef $USER;
    undef $PASSWORD;
}

sub clear_proxy_credentials {
    undef $PROXY_USER;
    undef $PROXY_PASSWORD;
}

sub get_proxy_credentials {
    my $self = shift;
    if ($PROXY_USER && $PROXY_PASSWORD) {
        return ($PROXY_USER, $PROXY_PASSWORD);
    }
    if ( defined $CPAN::Config->{proxy_user}
            && $CPAN::Config->{proxy_user}
    ) {
        $PROXY_USER = $CPAN::Config->{proxy_user};
        $PROXY_PASSWORD = $CPAN::Config->{proxy_pass} || "";
        return ($PROXY_USER, $PROXY_PASSWORD);
    }
    my $username_prompt = "\nProxy authentication needed!
 (Note: to permanently configure username and password run
   o conf proxy_user your_username
   o conf proxy_pass your_password
     )\nUsername:";
    ($PROXY_USER, $PROXY_PASSWORD) =
        _get_username_and_password_from_user($username_prompt);
    return ($PROXY_USER,$PROXY_PASSWORD);
}

sub get_non_proxy_credentials {
    my $self = shift;
    if ($USER && $PASSWORD) {
        return ($USER, $PASSWORD);
    }
    if ( defined $CPAN::Config->{username} ) {
        $USER = $CPAN::Config->{username};
        $PASSWORD = $CPAN::Config->{password} || "";
        return ($USER, $PASSWORD);
    }
    my $username_prompt = "\nAuthentication needed!
     (Note: to permanently configure username and password run
       o conf username your_username
       o conf password your_password
     )\nUsername:";

    ($USER, $PASSWORD) =
        _get_username_and_password_from_user($username_prompt);
    return ($USER,$PASSWORD);
}

sub _get_username_and_password_from_user {
    my $username_message = shift;
    my ($username,$password);

    ExtUtils::MakeMaker->import(qw(prompt));
    $username = prompt($username_message);
        if ($CPAN::META->has_inst("Term::ReadKey")) {
            Term::ReadKey::ReadMode("noecho");
        }
    else {
        $CPAN::Frontend->mywarn(
            "Warning: Term::ReadKey seems not to be available, your password will be echoed to the terminal!\n"
        );
    }
    $password = prompt("Password:");

        if ($CPAN::META->has_inst("Term::ReadKey")) {
            Term::ReadKey::ReadMode("restore");
        }
        $CPAN::Frontend->myprint("\n\n");
    return ($username,$password);
}

1;

