# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
# vim: ts=4 sts=4 sw=4:
package CPAN::Exception::yaml_not_installed;
use strict;
use overload '""' => "as_string";

use vars qw(
            $VERSION
);
$VERSION = "5.5";


sub new {
    my($class,$module,$file,$during) = @_;
    bless { module => $module, file => $file, during => $during }, $class;
}

sub as_string {
    my($self) = shift;
    "'$self->{module}' not installed, cannot $self->{during} '$self->{file}'\n";
}

1;
