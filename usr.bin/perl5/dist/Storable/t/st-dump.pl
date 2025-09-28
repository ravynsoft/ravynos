#
#  Copyright (c) 1995-2000, Raphael Manfredi
#  
#  You may redistribute only under the same terms as Perl 5, as specified
#  in the README file that comes with the distribution.
#

package dump;
use Carp;

%dump = (
	'SCALAR'	=> 'dump_scalar',
	'LVALUE'	=> 'dump_scalar',
	'ARRAY'		=> 'dump_array',
	'HASH'		=> 'dump_hash',
	'REF'		=> 'dump_ref',
);

# Given an object, dump its transitive data closure
sub main::dump {
	my ($object) = @_;
	croak "Not a reference!" unless ref($object);
	local %dumped;
	local %object;
	local $count = 0;
	local $dumped = '';
	&recursive_dump($object, 1);
	return $dumped;
}

# This is the root recursive dumping routine that may indirectly be
# called by one of the routine it calls...
# The link parameter is set to false when the reference passed to
# the routine is an internal temporary variable, implying the object's
# address is not to be dumped in the %dumped table since it's not a
# user-visible object.
sub recursive_dump {
	my ($object, $link) = @_;

	# Get something like SCALAR(0x...) or TYPE=SCALAR(0x...).
	# Then extract the bless, ref and address parts of that string.

	my $what = "$object";		# Stringify
	my ($bless, $ref, $addr) = $what =~ /^(\w+)=(\w+)\((0x.*)\)$/;
	($ref, $addr) = $what =~ /^(\w+)\((0x.*)\)$/ unless $bless;

	# Special case for references to references. When stringified,
	# they appear as being scalars. However, ref() correctly pinpoints
	# them as being references indirections. And that's it.

	$ref = 'REF' if ref($object) eq 'REF';

	# Make sure the object has not been already dumped before.
	# We don't want to duplicate data. Retrieval will know how to
	# relink from the previously seen object.

	if ($link && $dumped{$addr}++) {
		my $num = $object{$addr};
		$dumped .= "OBJECT #$num seen\n";
		return;
	}

	my $objcount = $count++;
	$object{$addr} = $objcount;

	# Call the appropriate dumping routine based on the reference type.
	# If the referenced was blessed, we bless it once the object is dumped.
	# The retrieval code will perform the same on the last object retrieved.

	croak "Unknown simple type '$ref'" unless defined $dump{$ref};

	&{$dump{$ref}}($object);	# Dump object
	&bless($bless) if $bless;	# Mark it as blessed, if necessary

	$dumped .= "OBJECT $objcount\n";
}

# Indicate that current object is blessed
sub bless {
	my ($class) = @_;
	$dumped .= "BLESS $class\n";
}

# Dump single scalar
sub dump_scalar {
	my ($sref) = @_;
	my $scalar = $$sref;
	unless (defined $scalar) {
		$dumped .= "UNDEF\n";
		return;
	}
	my $len = length($scalar);
	$dumped .= "SCALAR len=$len $scalar\n";
}

# Dump array
sub dump_array {
	my ($aref) = @_;
	my $items = 0 + @{$aref};
	$dumped .= "ARRAY items=$items\n";
	foreach $item (@{$aref}) {
		unless (defined $item) {
			$dumped .= 'ITEM_UNDEF' . "\n";
			next;
		}
		$dumped .= 'ITEM ';
		&recursive_dump(\$item, 1);
	}
}

# Dump hash table
sub dump_hash {
	my ($href) = @_;
	my $items = scalar(keys %{$href});
	$dumped .= "HASH items=$items\n";
	foreach $key (sort keys %{$href}) {
		$dumped .= 'KEY ';
		&recursive_dump(\$key, undef);
		unless (defined $href->{$key}) {
			$dumped .= 'VALUE_UNDEF' . "\n";
			next;
		}
		$dumped .= 'VALUE ';
		&recursive_dump(\$href->{$key}, 1);
	}
}

# Dump reference to reference
sub dump_ref {
	my ($rref) = @_;
	my $deref = $$rref;				# Follow reference to reference
	$dumped .= 'REF ';
	&recursive_dump($deref, 1);		# $dref is a reference
}

1;
