# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
# vim: ts=4 sts=4 sw=4:
package CPAN::Prompt;
use overload '""' => "as_string";
use vars qw($prompt);
use vars qw(
            $VERSION
);
$VERSION = "5.5";


$prompt = "cpan> ";
$CPAN::CurrentCommandId ||= 0;
sub new {
    bless {}, shift;
}
sub as_string {
    my $word = "cpan";
    unless ($CPAN::META->{LOCK}) {
        $word = "nolock_cpan";
    }
    if ($CPAN::Config->{commandnumber_in_prompt}) {
        sprintf "$word\[%d]> ", $CPAN::CurrentCommandId;
    } else {
        "$word> ";
    }
}

1;
