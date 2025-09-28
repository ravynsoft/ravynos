# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
# vim: ts=4 sts=4 sw=4:
package CPAN::Distrostatus;
use overload '""' => "as_string",
    fallback => 1;
use vars qw($something_has_failed_at);
use vars qw(
            $VERSION
);
$VERSION = "5.5";


sub new {
    my($class,$arg) = @_;
    my $failed = substr($arg,0,2) eq "NO";
    if ($failed) {
        $something_has_failed_at = $CPAN::CurrentCommandId;
    }
    bless {
           TEXT => $arg,
           FAILED => $failed,
           COMMANDID => $CPAN::CurrentCommandId,
           TIME => time,
          }, $class;
}
sub something_has_just_failed () {
    defined $something_has_failed_at &&
        $something_has_failed_at == $CPAN::CurrentCommandId;
}
sub commandid { shift->{COMMANDID} }
sub failed { shift->{FAILED} }
sub text {
    my($self,$set) = @_;
    if (defined $set) {
        $self->{TEXT} = $set;
    }
    $self->{TEXT};
}
sub as_string {
    my($self) = @_;
    $self->text;
}


1;
