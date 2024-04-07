# -*- Mode: cperl; coding: utf-8; cperl-indent-level: 4 -*-
# vim: ts=4 sts=4 sw=4:
package CPAN::Exception::yaml_process_error;
use strict;
use overload '""' => "as_string";

use vars qw(
            $VERSION
);
$VERSION = "5.5";


sub new {
    my($class,$module,$file,$during,$error) = @_;
    # my $at = Carp::longmess(""); # XXX find something more beautiful
    bless { module => $module,
            file => $file,
            during => $during,
            error => $error,
            # at => $at,
          }, $class;
}

sub as_string {
    my($self) = shift;
    if ($self->{during}) {
        if ($self->{file}) {
            if ($self->{module}) {
                if ($self->{error}) {
                    return "Alert: While trying to '$self->{during}' YAML file\n".
                        " '$self->{file}'\n".
                            "with '$self->{module}' the following error was encountered:\n".
                                "  $self->{error}\n";
                } else {
                    return "Alert: While trying to '$self->{during}' YAML file\n".
                        " '$self->{file}'\n".
                            "with '$self->{module}' some unknown error was encountered\n";
                }
            } else {
                return "Alert: While trying to '$self->{during}' YAML file\n".
                    " '$self->{file}'\n".
                        "some unknown error was encountered\n";
            }
        } else {
            return "Alert: While trying to '$self->{during}' some YAML file\n".
                    "some unknown error was encountered\n";
        }
    } else {
        return "Alert: unknown error encountered\n";
    }
}

1;
