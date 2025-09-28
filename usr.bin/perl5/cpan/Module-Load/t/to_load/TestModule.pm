package TestModule;

use strict;
use warnings;

require Exporter;
use vars qw(@EXPORT @EXPORT_OK @ISA $IMPORTED);

@ISA        = qw(Exporter);
@EXPORT     = qw(func2);
@EXPORT_OK  = qw(func1);

### test if import gets called properly
sub import   { $IMPORTED = 1; 
               ### this breaks on 5.8.[45] which have a bug with goto's losing
               ### arguments in @_. This is the cause of the 0.14 tester failures
               ### under 5.8.[45]. The bug is NOT in exporter, but core perl:
               ### http://testers.cpan.org/show/Module-Load.html
               #goto &Exporter::import; 
               
               ### instead, use the undocumented, but widely used $ExportLevel
               ### which will make sure we pass all arguments, and even works
               ### on buggy 5.8.[45]
               do { local $Exporter::ExportLevel += 1; Exporter::import(@_) }
             }
             
sub imported { $IMPORTED;       }

sub func1    { return "func1";  }

sub func2    { return "func2";  }

1;
