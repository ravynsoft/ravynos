package Descriptions;
$VERSION = '1.00';

use Attribute::Handlers;

my %name;

sub name {
	return $name{$_[2]}||*{$_[1]}{NAME};
}

sub UNIVERSAL::Name :ATTR {
	$name{$_[2]} = $_[4];
}

sub UNIVERSAL::Purpose :ATTR {
	print STDERR "Purpose of ", &name, " is $_[4]\n";
}

sub UNIVERSAL::Unit :ATTR {
	print STDERR &name, " measured in $_[4]\n";
}


1;
