package Dummy;

# Attempt to emulate a bug with finding the version in Exporter.
$VERSION = '5.562';

sub exclaim { "I CAN FROM " . __PACKAGE__ }

package Dummy::InlineChild;

sub exclaim { "I CAN FROM " . __PACKAGE__ }

1;
