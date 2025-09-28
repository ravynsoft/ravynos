use 5.006;
use strict;
use warnings;
package CPAN::Meta::Validator;

our $VERSION = '2.150010';

#pod =head1 SYNOPSIS
#pod
#pod   my $struct = decode_json_file('META.json');
#pod
#pod   my $cmv = CPAN::Meta::Validator->new( $struct );
#pod
#pod   unless ( $cmv->is_valid ) {
#pod     my $msg = "Invalid META structure.  Errors found:\n";
#pod     $msg .= join( "\n", $cmv->errors );
#pod     die $msg;
#pod   }
#pod
#pod =head1 DESCRIPTION
#pod
#pod This module validates a CPAN Meta structure against the version of the
#pod the specification claimed in the C<meta-spec> field of the structure.
#pod
#pod =cut

#--------------------------------------------------------------------------#
# This code copied and adapted from Test::CPAN::Meta
# by Barbie, <barbie@cpan.org> for Miss Barbell Productions,
# L<http://www.missbarbell.co.uk>
#--------------------------------------------------------------------------#

#--------------------------------------------------------------------------#
# Specification Definitions
#--------------------------------------------------------------------------#

my %known_specs = (
    '1.4' => 'http://module-build.sourceforge.net/META-spec-v1.4.html',
    '1.3' => 'http://module-build.sourceforge.net/META-spec-v1.3.html',
    '1.2' => 'http://module-build.sourceforge.net/META-spec-v1.2.html',
    '1.1' => 'http://module-build.sourceforge.net/META-spec-v1.1.html',
    '1.0' => 'http://module-build.sourceforge.net/META-spec-v1.0.html'
);
my %known_urls = map {$known_specs{$_} => $_} keys %known_specs;

my $module_map1 = { 'map' => { ':key' => { name => \&module, value => \&exversion } } };

my $module_map2 = { 'map' => { ':key' => { name => \&module, value => \&version   } } };

my $no_index_2 = {
    'map'       => { file       => { list => { value => \&string } },
                     directory  => { list => { value => \&string } },
                     'package'  => { list => { value => \&string } },
                     namespace  => { list => { value => \&string } },
                    ':key'      => { name => \&custom_2, value => \&anything },
    }
};

my $no_index_1_3 = {
    'map'       => { file       => { list => { value => \&string } },
                     directory  => { list => { value => \&string } },
                     'package'  => { list => { value => \&string } },
                     namespace  => { list => { value => \&string } },
                     ':key'     => { name => \&string, value => \&anything },
    }
};

my $no_index_1_2 = {
    'map'       => { file       => { list => { value => \&string } },
                     dir        => { list => { value => \&string } },
                     'package'  => { list => { value => \&string } },
                     namespace  => { list => { value => \&string } },
                     ':key'     => { name => \&string, value => \&anything },
    }
};

my $no_index_1_1 = {
    'map'       => { ':key'     => { name => \&string, list => { value => \&string } },
    }
};

my $prereq_map = {
  map => {
    ':key' => {
      name => \&phase,
      'map' => {
        ':key'  => {
          name => \&relation,
          %$module_map1,
        },
      },
    }
  },
};

my %definitions = (
  '2' => {
    # REQUIRED
    'abstract'            => { mandatory => 1, value => \&string  },
    'author'              => { mandatory => 1, list => { value => \&string } },
    'dynamic_config'      => { mandatory => 1, value => \&boolean },
    'generated_by'        => { mandatory => 1, value => \&string  },
    'license'             => { mandatory => 1, list => { value => \&license } },
    'meta-spec' => {
      mandatory => 1,
      'map' => {
        version => { mandatory => 1, value => \&version},
        url     => { value => \&url },
        ':key' => { name => \&custom_2, value => \&anything },
      }
    },
    'name'                => { mandatory => 1, value => \&string  },
    'release_status'      => { mandatory => 1, value => \&release_status },
    'version'             => { mandatory => 1, value => \&version },

    # OPTIONAL
    'description' => { value => \&string },
    'keywords'    => { list => { value => \&string } },
    'no_index'    => $no_index_2,
    'optional_features'   => {
      'map'       => {
        ':key'  => {
          name => \&string,
          'map'   => {
            description        => { value => \&string },
            prereqs => $prereq_map,
            ':key' => { name => \&custom_2, value => \&anything },
          }
        }
      }
    },
    'prereqs' => $prereq_map,
    'provides'    => {
      'map'       => {
        ':key' => {
          name  => \&module,
          'map' => {
            file    => { mandatory => 1, value => \&file },
            version => { value => \&version },
            ':key' => { name => \&custom_2, value => \&anything },
          }
        }
      }
    },
    'resources'   => {
      'map'       => {
        license    => { list => { value => \&url } },
        homepage   => { value => \&url },
        bugtracker => {
          'map' => {
            web => { value => \&url },
            mailto => { value => \&string},
            ':key' => { name => \&custom_2, value => \&anything },
          }
        },
        repository => {
          'map' => {
            web => { value => \&url },
            url => { value => \&url },
            type => { value => \&string },
            ':key' => { name => \&custom_2, value => \&anything },
          }
        },
        ':key'     => { value => \&string, name => \&custom_2 },
      }
    },

    # CUSTOM -- additional user defined key/value pairs
    # note we can only validate the key name, as the structure is user defined
    ':key'        => { name => \&custom_2, value => \&anything },
  },

'1.4' => {
  'meta-spec'           => {
    mandatory => 1,
    'map' => {
      version => { mandatory => 1, value => \&version},
      url     => { mandatory => 1, value => \&urlspec },
      ':key'  => { name => \&string, value => \&anything },
    },
  },

  'name'                => { mandatory => 1, value => \&string  },
  'version'             => { mandatory => 1, value => \&version },
  'abstract'            => { mandatory => 1, value => \&string  },
  'author'              => { mandatory => 1, list  => { value => \&string } },
  'license'             => { mandatory => 1, value => \&license },
  'generated_by'        => { mandatory => 1, value => \&string  },

  'distribution_type'   => { value => \&string  },
  'dynamic_config'      => { value => \&boolean },

  'requires'            => $module_map1,
  'recommends'          => $module_map1,
  'build_requires'      => $module_map1,
  'configure_requires'  => $module_map1,
  'conflicts'           => $module_map2,

  'optional_features'   => {
    'map'       => {
        ':key'  => { name => \&string,
            'map'   => { description        => { value => \&string },
                         requires           => $module_map1,
                         recommends         => $module_map1,
                         build_requires     => $module_map1,
                         conflicts          => $module_map2,
                         ':key'  => { name => \&string, value => \&anything },
            }
        }
     }
  },

  'provides'    => {
    'map'       => {
      ':key' => { name  => \&module,
        'map' => {
          file    => { mandatory => 1, value => \&file },
          version => { value => \&version },
          ':key'  => { name => \&string, value => \&anything },
        }
      }
    }
  },

  'no_index'    => $no_index_1_3,
  'private'     => $no_index_1_3,

  'keywords'    => { list => { value => \&string } },

  'resources'   => {
    'map'       => { license    => { value => \&url },
                     homepage   => { value => \&url },
                     bugtracker => { value => \&url },
                     repository => { value => \&url },
                     ':key'     => { value => \&string, name => \&custom_1 },
    }
  },

  # additional user defined key/value pairs
  # note we can only validate the key name, as the structure is user defined
  ':key'        => { name => \&string, value => \&anything },
},

'1.3' => {
  'meta-spec'           => {
    mandatory => 1,
    'map' => {
      version => { mandatory => 1, value => \&version},
      url     => { mandatory => 1, value => \&urlspec },
      ':key'  => { name => \&string, value => \&anything },
    },
  },

  'name'                => { mandatory => 1, value => \&string  },
  'version'             => { mandatory => 1, value => \&version },
  'abstract'            => { mandatory => 1, value => \&string  },
  'author'              => { mandatory => 1, list  => { value => \&string } },
  'license'             => { mandatory => 1, value => \&license },
  'generated_by'        => { mandatory => 1, value => \&string  },

  'distribution_type'   => { value => \&string  },
  'dynamic_config'      => { value => \&boolean },

  'requires'            => $module_map1,
  'recommends'          => $module_map1,
  'build_requires'      => $module_map1,
  'conflicts'           => $module_map2,

  'optional_features'   => {
    'map'       => {
        ':key'  => { name => \&string,
            'map'   => { description        => { value => \&string },
                         requires           => $module_map1,
                         recommends         => $module_map1,
                         build_requires     => $module_map1,
                         conflicts          => $module_map2,
                         ':key'  => { name => \&string, value => \&anything },
            }
        }
     }
  },

  'provides'    => {
    'map'       => {
      ':key' => { name  => \&module,
        'map' => {
          file    => { mandatory => 1, value => \&file },
          version => { value => \&version },
          ':key'  => { name => \&string, value => \&anything },
        }
      }
    }
  },


  'no_index'    => $no_index_1_3,
  'private'     => $no_index_1_3,

  'keywords'    => { list => { value => \&string } },

  'resources'   => {
    'map'       => { license    => { value => \&url },
                     homepage   => { value => \&url },
                     bugtracker => { value => \&url },
                     repository => { value => \&url },
                     ':key'     => { value => \&string, name => \&custom_1 },
    }
  },

  # additional user defined key/value pairs
  # note we can only validate the key name, as the structure is user defined
  ':key'        => { name => \&string, value => \&anything },
},

# v1.2 is misleading, it seems to assume that a number of fields where created
# within v1.1, when they were created within v1.2. This may have been an
# original mistake, and that a v1.1 was retro fitted into the timeline, when
# v1.2 was originally slated as v1.1. But I could be wrong ;)
'1.2' => {
  'meta-spec'           => {
    mandatory => 1,
    'map' => {
      version => { mandatory => 1, value => \&version},
      url     => { mandatory => 1, value => \&urlspec },
      ':key'  => { name => \&string, value => \&anything },
    },
  },


  'name'                => { mandatory => 1, value => \&string  },
  'version'             => { mandatory => 1, value => \&version },
  'license'             => { mandatory => 1, value => \&license },
  'generated_by'        => { mandatory => 1, value => \&string  },
  'author'              => { mandatory => 1, list => { value => \&string } },
  'abstract'            => { mandatory => 1, value => \&string  },

  'distribution_type'   => { value => \&string  },
  'dynamic_config'      => { value => \&boolean },

  'keywords'            => { list => { value => \&string } },

  'private'             => $no_index_1_2,
  '$no_index'           => $no_index_1_2,

  'requires'            => $module_map1,
  'recommends'          => $module_map1,
  'build_requires'      => $module_map1,
  'conflicts'           => $module_map2,

  'optional_features'   => {
    'map'       => {
        ':key'  => { name => \&string,
            'map'   => { description        => { value => \&string },
                         requires           => $module_map1,
                         recommends         => $module_map1,
                         build_requires     => $module_map1,
                         conflicts          => $module_map2,
                         ':key'  => { name => \&string, value => \&anything },
            }
        }
     }
  },

  'provides'    => {
    'map'       => {
      ':key' => { name  => \&module,
        'map' => {
          file    => { mandatory => 1, value => \&file },
          version => { value => \&version },
          ':key'  => { name => \&string, value => \&anything },
        }
      }
    }
  },

  'resources'   => {
    'map'       => { license    => { value => \&url },
                     homepage   => { value => \&url },
                     bugtracker => { value => \&url },
                     repository => { value => \&url },
                     ':key'     => { value => \&string, name => \&custom_1 },
    }
  },

  # additional user defined key/value pairs
  # note we can only validate the key name, as the structure is user defined
  ':key'        => { name => \&string, value => \&anything },
},

# note that the 1.1 spec only specifies 'version' as mandatory
'1.1' => {
  'name'                => { value => \&string  },
  'version'             => { mandatory => 1, value => \&version },
  'license'             => { value => \&license },
  'generated_by'        => { value => \&string  },

  'license_uri'         => { value => \&url },
  'distribution_type'   => { value => \&string  },
  'dynamic_config'      => { value => \&boolean },

  'private'             => $no_index_1_1,

  'requires'            => $module_map1,
  'recommends'          => $module_map1,
  'build_requires'      => $module_map1,
  'conflicts'           => $module_map2,

  # additional user defined key/value pairs
  # note we can only validate the key name, as the structure is user defined
  ':key'        => { name => \&string, value => \&anything },
},

# note that the 1.0 spec doesn't specify optional or mandatory fields
# but we will treat version as mandatory since otherwise META 1.0 is
# completely arbitrary and pointless
'1.0' => {
  'name'                => { value => \&string  },
  'version'             => { mandatory => 1, value => \&version },
  'license'             => { value => \&license },
  'generated_by'        => { value => \&string  },

  'license_uri'         => { value => \&url },
  'distribution_type'   => { value => \&string  },
  'dynamic_config'      => { value => \&boolean },

  'requires'            => $module_map1,
  'recommends'          => $module_map1,
  'build_requires'      => $module_map1,
  'conflicts'           => $module_map2,

  # additional user defined key/value pairs
  # note we can only validate the key name, as the structure is user defined
  ':key'        => { name => \&string, value => \&anything },
},
);

#--------------------------------------------------------------------------#
# Code
#--------------------------------------------------------------------------#

#pod =method new
#pod
#pod   my $cmv = CPAN::Meta::Validator->new( $struct )
#pod
#pod The constructor must be passed a metadata structure.
#pod
#pod =cut

sub new {
  my ($class,$data) = @_;

  # create an attributes hash
  my $self = {
    'data'    => $data,
    'spec'    => eval { $data->{'meta-spec'}{'version'} } || "1.0",
    'errors'  => undef,
  };

  # create the object
  return bless $self, $class;
}

#pod =method is_valid
#pod
#pod   if ( $cmv->is_valid ) {
#pod     ...
#pod   }
#pod
#pod Returns a boolean value indicating whether the metadata provided
#pod is valid.
#pod
#pod =cut

sub is_valid {
    my $self = shift;
    my $data = $self->{data};
    my $spec_version = $self->{spec};
    $self->check_map($definitions{$spec_version},$data);
    return ! $self->errors;
}

#pod =method errors
#pod
#pod   warn( join "\n", $cmv->errors );
#pod
#pod Returns a list of errors seen during validation.
#pod
#pod =cut

sub errors {
    my $self = shift;
    return ()   unless(defined $self->{errors});
    return @{$self->{errors}};
}

#pod =begin :internals
#pod
#pod =head2 Check Methods
#pod
#pod =over
#pod
#pod =item *
#pod
#pod check_map($spec,$data)
#pod
#pod Checks whether a map (or hash) part of the data structure conforms to the
#pod appropriate specification definition.
#pod
#pod =item *
#pod
#pod check_list($spec,$data)
#pod
#pod Checks whether a list (or array) part of the data structure conforms to
#pod the appropriate specification definition.
#pod
#pod =item *
#pod
#pod =back
#pod
#pod =cut

my $spec_error = "Missing validation action in specification. "
  . "Must be one of 'map', 'list', or 'value'";

sub check_map {
    my ($self,$spec,$data) = @_;

    if(ref($spec) ne 'HASH') {
        $self->_error( "Unknown META specification, cannot validate." );
        return;
    }

    if(ref($data) ne 'HASH') {
        $self->_error( "Expected a map structure from string or file." );
        return;
    }

    for my $key (keys %$spec) {
        next    unless($spec->{$key}->{mandatory});
        next    if(defined $data->{$key});
        push @{$self->{stack}}, $key;
        $self->_error( "Missing mandatory field, '$key'" );
        pop @{$self->{stack}};
    }

    for my $key (keys %$data) {
        push @{$self->{stack}}, $key;
        if($spec->{$key}) {
            if($spec->{$key}{value}) {
                $spec->{$key}{value}->($self,$key,$data->{$key});
            } elsif($spec->{$key}{'map'}) {
                $self->check_map($spec->{$key}{'map'},$data->{$key});
            } elsif($spec->{$key}{'list'}) {
                $self->check_list($spec->{$key}{'list'},$data->{$key});
            } else {
                $self->_error( "$spec_error for '$key'" );
            }

        } elsif ($spec->{':key'}) {
            $spec->{':key'}{name}->($self,$key,$key);
            if($spec->{':key'}{value}) {
                $spec->{':key'}{value}->($self,$key,$data->{$key});
            } elsif($spec->{':key'}{'map'}) {
                $self->check_map($spec->{':key'}{'map'},$data->{$key});
            } elsif($spec->{':key'}{'list'}) {
                $self->check_list($spec->{':key'}{'list'},$data->{$key});
            } else {
                $self->_error( "$spec_error for ':key'" );
            }


        } else {
            $self->_error( "Unknown key, '$key', found in map structure" );
        }
        pop @{$self->{stack}};
    }
}

sub check_list {
    my ($self,$spec,$data) = @_;

    if(ref($data) ne 'ARRAY') {
        $self->_error( "Expected a list structure" );
        return;
    }

    if(defined $spec->{mandatory}) {
        if(!defined $data->[0]) {
            $self->_error( "Missing entries from mandatory list" );
        }
    }

    for my $value (@$data) {
        push @{$self->{stack}}, $value || "<undef>";
        if(defined $spec->{value}) {
            $spec->{value}->($self,'list',$value);
        } elsif(defined $spec->{'map'}) {
            $self->check_map($spec->{'map'},$value);
        } elsif(defined $spec->{'list'}) {
            $self->check_list($spec->{'list'},$value);
        } elsif ($spec->{':key'}) {
            $self->check_map($spec,$value);
        } else {
          $self->_error( "$spec_error associated with '$self->{stack}[-2]'" );
        }
        pop @{$self->{stack}};
    }
}

#pod =head2 Validator Methods
#pod
#pod =over
#pod
#pod =item *
#pod
#pod header($self,$key,$value)
#pod
#pod Validates that the header is valid.
#pod
#pod Note: No longer used as we now read the data structure, not the file.
#pod
#pod =item *
#pod
#pod url($self,$key,$value)
#pod
#pod Validates that a given value is in an acceptable URL format
#pod
#pod =item *
#pod
#pod urlspec($self,$key,$value)
#pod
#pod Validates that the URL to a META specification is a known one.
#pod
#pod =item *
#pod
#pod string_or_undef($self,$key,$value)
#pod
#pod Validates that the value is either a string or an undef value. Bit of a
#pod catchall function for parts of the data structure that are completely user
#pod defined.
#pod
#pod =item *
#pod
#pod string($self,$key,$value)
#pod
#pod Validates that a string exists for the given key.
#pod
#pod =item *
#pod
#pod file($self,$key,$value)
#pod
#pod Validate that a file is passed for the given key. This may be made more
#pod thorough in the future. For now it acts like \&string.
#pod
#pod =item *
#pod
#pod exversion($self,$key,$value)
#pod
#pod Validates a list of versions, e.g. '<= 5, >=2, ==3, !=4, >1, <6, 0'.
#pod
#pod =item *
#pod
#pod version($self,$key,$value)
#pod
#pod Validates a single version string. Versions of the type '5.8.8' and '0.00_00'
#pod are both valid. A leading 'v' like 'v1.2.3' is also valid.
#pod
#pod =item *
#pod
#pod boolean($self,$key,$value)
#pod
#pod Validates for a boolean value: a defined value that is either "1" or "0" or
#pod stringifies to those values.
#pod
#pod =item *
#pod
#pod license($self,$key,$value)
#pod
#pod Validates that a value is given for the license. Returns 1 if an known license
#pod type, or 2 if a value is given but the license type is not a recommended one.
#pod
#pod =item *
#pod
#pod custom_1($self,$key,$value)
#pod
#pod Validates that the given key is in CamelCase, to indicate a user defined
#pod keyword and only has characters in the class [-_a-zA-Z].  In version 1.X
#pod of the spec, this was only explicitly stated for 'resources'.
#pod
#pod =item *
#pod
#pod custom_2($self,$key,$value)
#pod
#pod Validates that the given key begins with 'x_' or 'X_', to indicate a user
#pod defined keyword and only has characters in the class [-_a-zA-Z]
#pod
#pod =item *
#pod
#pod identifier($self,$key,$value)
#pod
#pod Validates that key is in an acceptable format for the META specification,
#pod for an identifier, i.e. any that matches the regular expression
#pod qr/[a-z][a-z_]/i.
#pod
#pod =item *
#pod
#pod module($self,$key,$value)
#pod
#pod Validates that a given key is in an acceptable module name format, e.g.
#pod 'Test::CPAN::Meta::Version'.
#pod
#pod =back
#pod
#pod =end :internals
#pod
#pod =cut

sub header {
    my ($self,$key,$value) = @_;
    if(defined $value) {
        return 1    if($value && $value =~ /^--- #YAML:1.0/);
    }
    $self->_error( "file does not have a valid YAML header." );
    return 0;
}

sub release_status {
  my ($self,$key,$value) = @_;
  if(defined $value) {
    my $version = $self->{data}{version} || '';
    if ( $version =~ /_/ ) {
      return 1 if ( $value =~ /\A(?:testing|unstable)\z/ );
      $self->_error( "'$value' for '$key' is invalid for version '$version'" );
    }
    else {
      return 1 if ( $value =~ /\A(?:stable|testing|unstable)\z/ );
      $self->_error( "'$value' for '$key' is invalid" );
    }
  }
  else {
    $self->_error( "'$key' is not defined" );
  }
  return 0;
}

# _uri_split taken from URI::Split by Gisle Aas, Copyright 2003
sub _uri_split {
     return $_[0] =~ m,(?:([^:/?#]+):)?(?://([^/?#]*))?([^?#]*)(?:\?([^#]*))?(?:#(.*))?,;
}

sub url {
    my ($self,$key,$value) = @_;
    if(defined $value) {
      my ($scheme, $auth, $path, $query, $frag) = _uri_split($value);
      unless ( defined $scheme && length $scheme ) {
        $self->_error( "'$value' for '$key' does not have a URL scheme" );
        return 0;
      }
      unless ( defined $auth && length $auth ) {
        $self->_error( "'$value' for '$key' does not have a URL authority" );
        return 0;
      }
      return 1;
    }
    $value ||= '';
    $self->_error( "'$value' for '$key' is not a valid URL." );
    return 0;
}

sub urlspec {
    my ($self,$key,$value) = @_;
    if(defined $value) {
        return 1    if($value && $known_specs{$self->{spec}} eq $value);
        if($value && $known_urls{$value}) {
            $self->_error( 'META specification URL does not match version' );
            return 0;
        }
    }
    $self->_error( 'Unknown META specification' );
    return 0;
}

sub anything { return 1 }

sub string {
    my ($self,$key,$value) = @_;
    if(defined $value) {
        return 1    if($value || $value =~ /^0$/);
    }
    $self->_error( "value is an undefined string" );
    return 0;
}

sub string_or_undef {
    my ($self,$key,$value) = @_;
    return 1    unless(defined $value);
    return 1    if($value || $value =~ /^0$/);
    $self->_error( "No string defined for '$key'" );
    return 0;
}

sub file {
    my ($self,$key,$value) = @_;
    return 1    if(defined $value);
    $self->_error( "No file defined for '$key'" );
    return 0;
}

sub exversion {
    my ($self,$key,$value) = @_;
    if(defined $value && ($value || $value =~ /0/)) {
        my $pass = 1;
        for(split(",",$value)) { $self->version($key,$_) or ($pass = 0); }
        return $pass;
    }
    $value = '<undef>'  unless(defined $value);
    $self->_error( "'$value' for '$key' is not a valid version." );
    return 0;
}

sub version {
    my ($self,$key,$value) = @_;
    if(defined $value) {
        return 0    unless($value || $value =~ /0/);
        return 1    if($value =~ /^\s*((<|<=|>=|>|!=|==)\s*)?v?\d+((\.\d+((_|\.)\d+)?)?)/);
    } else {
        $value = '<undef>';
    }
    $self->_error( "'$value' for '$key' is not a valid version." );
    return 0;
}

sub boolean {
    my ($self,$key,$value) = @_;
    if(defined $value) {
        return 1    if($value =~ /^(0|1)$/);
    } else {
        $value = '<undef>';
    }
    $self->_error( "'$value' for '$key' is not a boolean value." );
    return 0;
}

my %v1_licenses = (
    'perl'         => 'http://dev.perl.org/licenses/',
    'gpl'          => 'http://www.opensource.org/licenses/gpl-license.php',
    'apache'       => 'http://apache.org/licenses/LICENSE-2.0',
    'artistic'     => 'http://opensource.org/licenses/artistic-license.php',
    'artistic_2'   => 'http://opensource.org/licenses/artistic-license-2.0.php',
    'lgpl'         => 'http://www.opensource.org/licenses/lgpl-license.php',
    'bsd'          => 'http://www.opensource.org/licenses/bsd-license.php',
    'gpl'          => 'http://www.opensource.org/licenses/gpl-license.php',
    'mit'          => 'http://opensource.org/licenses/mit-license.php',
    'mozilla'      => 'http://opensource.org/licenses/mozilla1.1.php',
    'open_source'  => undef,
    'unrestricted' => undef,
    'restrictive'  => undef,
    'unknown'      => undef,
);

my %v2_licenses = map { $_ => 1 } qw(
  agpl_3
  apache_1_1
  apache_2_0
  artistic_1
  artistic_2
  bsd
  freebsd
  gfdl_1_2
  gfdl_1_3
  gpl_1
  gpl_2
  gpl_3
  lgpl_2_1
  lgpl_3_0
  mit
  mozilla_1_0
  mozilla_1_1
  openssl
  perl_5
  qpl_1_0
  ssleay
  sun
  zlib
  open_source
  restricted
  unrestricted
  unknown
);

sub license {
    my ($self,$key,$value) = @_;
    my $licenses = $self->{spec} < 2 ? \%v1_licenses : \%v2_licenses;
    if(defined $value) {
        return 1    if($value && exists $licenses->{$value});
    } else {
        $value = '<undef>';
    }
    $self->_error( "License '$value' is invalid" );
    return 0;
}

sub custom_1 {
    my ($self,$key) = @_;
    if(defined $key) {
        # a valid user defined key should be alphabetic
        # and contain at least one capital case letter.
        return 1    if($key && $key =~ /^[_a-z]+$/i && $key =~ /[A-Z]/);
    } else {
        $key = '<undef>';
    }
    $self->_error( "Custom resource '$key' must be in CamelCase." );
    return 0;
}

sub custom_2 {
    my ($self,$key) = @_;
    if(defined $key) {
        return 1    if($key && $key =~ /^x_/i);  # user defined
    } else {
        $key = '<undef>';
    }
    $self->_error( "Custom key '$key' must begin with 'x_' or 'X_'." );
    return 0;
}

sub identifier {
    my ($self,$key) = @_;
    if(defined $key) {
        return 1    if($key && $key =~ /^([a-z][_a-z]+)$/i);    # spec 2.0 defined
    } else {
        $key = '<undef>';
    }
    $self->_error( "Key '$key' is not a legal identifier." );
    return 0;
}

sub module {
    my ($self,$key) = @_;
    if(defined $key) {
        return 1    if($key && $key =~ /^[A-Za-z0-9_]+(::[A-Za-z0-9_]+)*$/);
    } else {
        $key = '<undef>';
    }
    $self->_error( "Key '$key' is not a legal module name." );
    return 0;
}

my @valid_phases = qw/ configure build test runtime develop /;
sub phase {
    my ($self,$key) = @_;
    if(defined $key) {
        return 1 if( length $key && grep { $key eq $_ } @valid_phases );
        return 1 if $key =~ /x_/i;
    } else {
        $key = '<undef>';
    }
    $self->_error( "Key '$key' is not a legal phase." );
    return 0;
}

my @valid_relations = qw/ requires recommends suggests conflicts /;
sub relation {
    my ($self,$key) = @_;
    if(defined $key) {
        return 1 if( length $key && grep { $key eq $_ } @valid_relations );
        return 1 if $key =~ /x_/i;
    } else {
        $key = '<undef>';
    }
    $self->_error( "Key '$key' is not a legal prereq relationship." );
    return 0;
}

sub _error {
    my $self = shift;
    my $mess = shift;

    $mess .= ' ('.join(' -> ',@{$self->{stack}}).')'  if($self->{stack});
    $mess .= " [Validation: $self->{spec}]";

    push @{$self->{errors}}, $mess;
}

1;

# ABSTRACT: validate CPAN distribution metadata structures

=pod

=encoding UTF-8

=head1 NAME

CPAN::Meta::Validator - validate CPAN distribution metadata structures

=head1 VERSION

version 2.150010

=head1 SYNOPSIS

  my $struct = decode_json_file('META.json');

  my $cmv = CPAN::Meta::Validator->new( $struct );

  unless ( $cmv->is_valid ) {
    my $msg = "Invalid META structure.  Errors found:\n";
    $msg .= join( "\n", $cmv->errors );
    die $msg;
  }

=head1 DESCRIPTION

This module validates a CPAN Meta structure against the version of the
the specification claimed in the C<meta-spec> field of the structure.

=head1 METHODS

=head2 new

  my $cmv = CPAN::Meta::Validator->new( $struct )

The constructor must be passed a metadata structure.

=head2 is_valid

  if ( $cmv->is_valid ) {
    ...
  }

Returns a boolean value indicating whether the metadata provided
is valid.

=head2 errors

  warn( join "\n", $cmv->errors );

Returns a list of errors seen during validation.

=begin :internals

=head2 Check Methods

=over

=item *

check_map($spec,$data)

Checks whether a map (or hash) part of the data structure conforms to the
appropriate specification definition.

=item *

check_list($spec,$data)

Checks whether a list (or array) part of the data structure conforms to
the appropriate specification definition.

=item *

=back

=head2 Validator Methods

=over

=item *

header($self,$key,$value)

Validates that the header is valid.

Note: No longer used as we now read the data structure, not the file.

=item *

url($self,$key,$value)

Validates that a given value is in an acceptable URL format

=item *

urlspec($self,$key,$value)

Validates that the URL to a META specification is a known one.

=item *

string_or_undef($self,$key,$value)

Validates that the value is either a string or an undef value. Bit of a
catchall function for parts of the data structure that are completely user
defined.

=item *

string($self,$key,$value)

Validates that a string exists for the given key.

=item *

file($self,$key,$value)

Validate that a file is passed for the given key. This may be made more
thorough in the future. For now it acts like \&string.

=item *

exversion($self,$key,$value)

Validates a list of versions, e.g. '<= 5, >=2, ==3, !=4, >1, <6, 0'.

=item *

version($self,$key,$value)

Validates a single version string. Versions of the type '5.8.8' and '0.00_00'
are both valid. A leading 'v' like 'v1.2.3' is also valid.

=item *

boolean($self,$key,$value)

Validates for a boolean value: a defined value that is either "1" or "0" or
stringifies to those values.

=item *

license($self,$key,$value)

Validates that a value is given for the license. Returns 1 if an known license
type, or 2 if a value is given but the license type is not a recommended one.

=item *

custom_1($self,$key,$value)

Validates that the given key is in CamelCase, to indicate a user defined
keyword and only has characters in the class [-_a-zA-Z].  In version 1.X
of the spec, this was only explicitly stated for 'resources'.

=item *

custom_2($self,$key,$value)

Validates that the given key begins with 'x_' or 'X_', to indicate a user
defined keyword and only has characters in the class [-_a-zA-Z]

=item *

identifier($self,$key,$value)

Validates that key is in an acceptable format for the META specification,
for an identifier, i.e. any that matches the regular expression
qr/[a-z][a-z_]/i.

=item *

module($self,$key,$value)

Validates that a given key is in an acceptable module name format, e.g.
'Test::CPAN::Meta::Version'.

=back

=end :internals

=for Pod::Coverage anything boolean check_list custom_1 custom_2 exversion file
identifier license module phase relation release_status string string_or_undef
url urlspec version header check_map

=head1 BUGS

Please report any bugs or feature using the CPAN Request Tracker.
Bugs can be submitted through the web interface at
L<http://rt.cpan.org/Dist/Display.html?Queue=CPAN-Meta>

When submitting a bug or request, please include a test-file or a patch to an
existing test-file that illustrates the bug or desired feature.

=head1 AUTHORS

=over 4

=item *

David Golden <dagolden@cpan.org>

=item *

Ricardo Signes <rjbs@cpan.org>

=item *

Adam Kennedy <adamk@cpan.org>

=back

=head1 COPYRIGHT AND LICENSE

This software is copyright (c) 2010 by David Golden, Ricardo Signes, Adam Kennedy and Contributors.

This is free software; you can redistribute it and/or modify it under
the same terms as the Perl 5 programming language system itself.

=cut

__END__


# vim: ts=2 sts=2 sw=2 et :
