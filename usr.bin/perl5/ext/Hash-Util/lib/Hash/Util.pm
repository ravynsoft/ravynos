package Hash::Util;

require 5.007003;
use strict;
use Carp;
use warnings;
no warnings 'uninitialized';
use warnings::register;
no warnings 'experimental::builtin';
use builtin qw(reftype);

require Exporter;
our @EXPORT_OK  = qw(
                     fieldhash fieldhashes

                     all_keys
                     lock_keys unlock_keys
                     lock_value unlock_value
                     lock_hash unlock_hash
                     lock_keys_plus
                     hash_locked hash_unlocked
                     hashref_locked hashref_unlocked
                     hidden_keys legal_keys

                     lock_ref_keys unlock_ref_keys
                     lock_ref_value unlock_ref_value
                     lock_hashref unlock_hashref
                     lock_ref_keys_plus
                     hidden_ref_keys legal_ref_keys

                     hash_seed hash_value hv_store
                     bucket_stats bucket_stats_formatted bucket_info bucket_array
                     lock_hash_recurse unlock_hash_recurse
                     lock_hashref_recurse unlock_hashref_recurse

                     hash_traversal_mask

                     bucket_ratio
                     used_buckets
                     num_buckets
                    );
BEGIN {
    # make sure all our XS routines are available early so their prototypes
    # are correctly applied in the following code.
    our $VERSION = '0.30';
    require XSLoader;
    XSLoader::load();
}

sub import {
    my $class = shift;
    if ( grep /fieldhash/, @_ ) {
        require Hash::Util::FieldHash;
        Hash::Util::FieldHash->import(':all'); # for re-export
    }
    unshift @_, $class;
    goto &Exporter::import;
}


=head1 NAME

Hash::Util - A selection of general-utility hash subroutines

=head1 SYNOPSIS

  # Restricted hashes

  use Hash::Util qw(
                     fieldhash fieldhashes

                     all_keys
                     lock_keys unlock_keys
                     lock_value unlock_value
                     lock_hash unlock_hash
                     lock_keys_plus
                     hash_locked hash_unlocked
                     hashref_locked hashref_unlocked
                     hidden_keys legal_keys

                     lock_ref_keys unlock_ref_keys
                     lock_ref_value unlock_ref_value
                     lock_hashref unlock_hashref
                     lock_ref_keys_plus
                     hidden_ref_keys legal_ref_keys

                     hash_seed hash_value hv_store
                     bucket_stats bucket_info bucket_array
                     lock_hash_recurse unlock_hash_recurse
                     lock_hashref_recurse unlock_hashref_recurse

                     hash_traversal_mask
                   );

  my %hash = (foo => 42, bar => 23);
  # Ways to restrict a hash
  lock_keys(%hash);
  lock_keys(%hash, @keyset);
  lock_keys_plus(%hash, @additional_keys);

  # Ways to inspect the properties of a restricted hash
  my @legal = legal_keys(%hash);
  my @hidden = hidden_keys(%hash);
  my $ref = all_keys(%hash,@keys,@hidden);
  my $is_locked = hash_locked(%hash);

  # Remove restrictions on the hash
  unlock_keys(%hash);

  # Lock individual values in a hash
  lock_value  (%hash, 'foo');
  unlock_value(%hash, 'foo');

  # Ways to change the restrictions on both keys and values
  lock_hash  (%hash);
  unlock_hash(%hash);

  my $hashes_are_randomised = hash_seed() !~ /^\0+$/;

  my $int_hash_value = hash_value( 'string' );

  my $mask= hash_traversal_mask(%hash);

  hash_traversal_mask(%hash,1234);

=head1 DESCRIPTION

C<Hash::Util> and C<Hash::Util::FieldHash> contain special functions
for manipulating hashes that don't really warrant a keyword.

C<Hash::Util> contains a set of functions that support
L<restricted hashes|/"Restricted hashes">. These are described in
this document.  C<Hash::Util::FieldHash> contains an (unrelated)
set of functions that support the use of hashes in
I<inside-out classes>, described in L<Hash::Util::FieldHash>.

By default C<Hash::Util> does not export anything.

=head2 Restricted hashes

5.8.0 introduces the ability to restrict a hash to a certain set of
keys.  No keys outside of this set can be added.  It also introduces
the ability to lock an individual key so it cannot be deleted and the
ability to ensure that an individual value cannot be changed.

This is intended to largely replace the deprecated pseudo-hashes.

=over 4

=item B<lock_keys>

=item B<unlock_keys>

  lock_keys(%hash);
  lock_keys(%hash, @keys);

Restricts the given %hash's set of keys to @keys.  If @keys is not
given it restricts it to its current keyset.  No more keys can be
added. delete() and exists() will still work, but will not alter
the set of allowed keys. B<Note>: the current implementation prevents
the hash from being bless()ed while it is in a locked state. Any attempt
to do so will raise an exception. Of course you can still bless()
the hash before you call lock_keys() so this shouldn't be a problem.

  unlock_keys(%hash);

Removes the restriction on the %hash's keyset.

B<Note> that if any of the values of the hash have been locked they will not
be unlocked after this sub executes.

Both routines return a reference to the hash operated on.

=cut

sub lock_ref_keys {
    my($hash, @keys) = @_;

    _clear_placeholders(%$hash);
    if( @keys ) {
        my %keys = map { ($_ => 1) } @keys;
        my %original_keys = map { ($_ => 1) } keys %$hash;
        foreach my $k (keys %original_keys) {
            croak "Hash has key '$k' which is not in the new key set"
              unless $keys{$k};
        }

        foreach my $k (@keys) {
            $hash->{$k} = undef unless exists $hash->{$k};
        }
        Internals::SvREADONLY %$hash, 1;

        foreach my $k (@keys) {
            delete $hash->{$k} unless $original_keys{$k};
        }
    }
    else {
        Internals::SvREADONLY %$hash, 1;
    }

    return $hash;
}

sub unlock_ref_keys {
    my $hash = shift;

    Internals::SvREADONLY %$hash, 0;
    return $hash;
}

sub   lock_keys (\%;@) {   lock_ref_keys(@_) }
sub unlock_keys (\%)   { unlock_ref_keys(@_) }

#=item B<_clear_placeholders>
#
# This function removes any placeholder keys from a hash. See Perl_hv_clear_placeholders()
# in hv.c for what it does exactly. It is currently exposed as XS by universal.c and
# injected into the Hash::Util namespace.
#
# It is not intended for use outside of this module, and may be changed
# or removed without notice or deprecation cycle.
#
#=cut
#
# sub _clear_placeholders {} # just in case someone searches...

=item B<lock_keys_plus>

  lock_keys_plus(%hash,@additional_keys)

Similar to C<lock_keys()>, with the difference being that the optional key list
specifies keys that may or may not be already in the hash. Essentially this is
an easier way to say

  lock_keys(%hash,@additional_keys,keys %hash);

Returns a reference to %hash

=cut


sub lock_ref_keys_plus {
    my ($hash,@keys) = @_;
    my @delete;
    _clear_placeholders(%$hash);
    foreach my $key (@keys) {
        unless (exists($hash->{$key})) {
            $hash->{$key}=undef;
            push @delete,$key;
        }
    }
    Internals::SvREADONLY(%$hash,1);
    delete @{$hash}{@delete};
    return $hash
}

sub lock_keys_plus(\%;@) { lock_ref_keys_plus(@_) }


=item B<lock_value>

=item B<unlock_value>

  lock_value  (%hash, $key);
  unlock_value(%hash, $key);

Locks and unlocks the value for an individual key of a hash.  The value of a
locked key cannot be changed.

Unless %hash has already been locked the key/value could be deleted
regardless of this setting.

Returns a reference to the %hash.

=cut

sub lock_ref_value {
    my($hash, $key) = @_;
    # I'm doubtful about this warning, as it seems not to be true.
    # Marking a value in the hash as RO is useful, regardless
    # of the status of the hash itself.
    carp "Cannot usefully lock values in an unlocked hash"
      if !Internals::SvREADONLY(%$hash) && warnings::enabled;
    Internals::SvREADONLY $hash->{$key}, 1;
    return $hash
}

sub unlock_ref_value {
    my($hash, $key) = @_;
    Internals::SvREADONLY $hash->{$key}, 0;
    return $hash
}

sub   lock_value (\%$) {   lock_ref_value(@_) }
sub unlock_value (\%$) { unlock_ref_value(@_) }


=item B<lock_hash>

=item B<unlock_hash>

    lock_hash(%hash);

lock_hash() locks an entire hash, making all keys and values read-only.
No value can be changed, no keys can be added or deleted.

    unlock_hash(%hash);

unlock_hash() does the opposite of lock_hash().  All keys and values
are made writable.  All values can be changed and keys can be added
and deleted.

Returns a reference to the %hash.

=cut

sub lock_hashref {
    my $hash = shift;

    lock_ref_keys($hash);

    foreach my $value (values %$hash) {
        Internals::SvREADONLY($value,1);
    }

    return $hash;
}

sub unlock_hashref {
    my $hash = shift;

    foreach my $value (values %$hash) {
        Internals::SvREADONLY($value, 0);
    }

    unlock_ref_keys($hash);

    return $hash;
}

sub   lock_hash (\%) {   lock_hashref(@_) }
sub unlock_hash (\%) { unlock_hashref(@_) }

=item B<lock_hash_recurse>

=item B<unlock_hash_recurse>

    lock_hash_recurse(%hash);

lock_hash() locks an entire hash and any hashes it references recursively,
making all keys and values read-only. No value can be changed, no keys can
be added or deleted.

This method B<only> recurses into hashes that are referenced by another hash.
Thus a Hash of Hashes (HoH) will all be restricted, but a Hash of Arrays of
Hashes (HoAoH) will only have the top hash restricted.

    unlock_hash_recurse(%hash);

unlock_hash_recurse() does the opposite of lock_hash_recurse().  All keys and
values are made writable.  All values can be changed and keys can be added
and deleted. Identical recursion restrictions apply as to lock_hash_recurse().

Returns a reference to the %hash.

=cut

sub lock_hashref_recurse {
    my $hash = shift;

    lock_ref_keys($hash);
    foreach my $value (values %$hash) {
        my $type = reftype($value);
        if (defined($type) and $type eq 'HASH') {
            lock_hashref_recurse($value);
        }
        Internals::SvREADONLY($value,1);
    }
    return $hash
}

sub unlock_hashref_recurse {
    my $hash = shift;

    foreach my $value (values %$hash) {
        my $type = reftype($value);
        if (defined($type) and $type eq 'HASH') {
            unlock_hashref_recurse($value);
        }
        Internals::SvREADONLY($value,0);
    }
    unlock_ref_keys($hash);
    return $hash;
}

sub   lock_hash_recurse (\%) {   lock_hashref_recurse(@_) }
sub unlock_hash_recurse (\%) { unlock_hashref_recurse(@_) }

=item B<hashref_locked>

=item B<hash_locked>

  hashref_locked(\%hash) and print "Hash is locked!\n";
  hash_locked(%hash) and print "Hash is locked!\n";

Returns true if the hash and its keys are locked.

=cut

sub hashref_locked {
    my $hash=shift;
    Internals::SvREADONLY(%$hash);
}

sub hash_locked(\%) { hashref_locked(@_) }

=item B<hashref_unlocked>

=item B<hash_unlocked>

  hashref_unlocked(\%hash) and print "Hash is unlocked!\n";
  hash_unlocked(%hash) and print "Hash is unlocked!\n";

Returns true if the hash and its keys are unlocked.

=cut

sub hashref_unlocked {
    my $hash=shift;
    !Internals::SvREADONLY(%$hash);
}

sub hash_unlocked(\%) { hashref_unlocked(@_) }

=for demerphqs_editor
sub legal_ref_keys{}
sub hidden_ref_keys{}
sub all_keys{}

=cut

sub legal_keys(\%) { legal_ref_keys(@_)  }
sub hidden_keys(\%){ hidden_ref_keys(@_) }

=item B<legal_keys>

  my @keys = legal_keys(%hash);

Returns the list of the keys that are legal in a restricted hash.
In the case of an unrestricted hash this is identical to calling
keys(%hash).

=item B<hidden_keys>

  my @keys = hidden_keys(%hash);

Returns the list of the keys that are legal in a restricted hash but
do not have a value associated to them. Thus if 'foo' is a
"hidden" key of the %hash it will return false for both C<defined>
and C<exists> tests.

In the case of an unrestricted hash this will return an empty list.

B<NOTE> this is an experimental feature that is heavily dependent
on the current implementation of restricted hashes. Should the
implementation change, this routine may become meaningless, in which
case it will return an empty list.

=item B<all_keys>

  all_keys(%hash,@keys,@hidden);

Populates the arrays @keys with the all the keys that would pass
an C<exists> tests, and populates @hidden with the remaining legal
keys that have not been utilized.

Returns a reference to the hash.

In the case of an unrestricted hash this will be equivalent to

  $ref = do {
      @keys = keys %hash;
      @hidden = ();
      \%hash
  };

B<NOTE> this is an experimental feature that is heavily dependent
on the current implementation of restricted hashes. Should the
implementation change this routine may become meaningless in which
case it will behave identically to how it would behave on an
unrestricted hash.

=item B<hash_seed>

    my $hash_seed = hash_seed();

hash_seed() returns the seed bytes used to randomise hash ordering.

B<Note that the hash seed is sensitive information>: by knowing it one
can craft a denial-of-service attack against Perl code, even remotely,
see L<perlsec/"Algorithmic Complexity Attacks"> for more information.
B<Do not disclose the hash seed> to people who don't need to know it.
See also L<perlrun/PERL_HASH_SEED_DEBUG>.

Prior to Perl 5.17.6 this function returned a UV, it now returns a string,
which may be of nearly any size as determined by the hash function your
Perl has been built with. Possible sizes may be but are not limited to
4 bytes (for most hash algorithms) and 16 bytes (for siphash).

=item B<hash_value>

    my $hash_value = hash_value($string);
    my $hash_value = hash_value($string, $seed);

C<hash_value($string)>
returns
the current perl's internal hash value for a given string.
C<hash_value($string, $seed)>
returns the hash value as if computed with a different seed.
If the custom seed is too short, the function errors out.
The minimum length of the seed is implementation-dependent.

Returns a 32-bit integer
representing the hash value of the string passed in.
The 1-parameter value is only reliable
for the lifetime of the process.
It may be different
depending on invocation, environment variables, perl version,
architectures, and build options.

B<Note that the hash value of a given string is sensitive information>:
by knowing it one can deduce the hash seed which in turn can allow one to
craft a denial-of-service attack against Perl code, even remotely,
see L<perlsec/"Algorithmic Complexity Attacks"> for more information.
B<Do not disclose the hash value of a string> to people who don't need to
know it. See also L<perlrun/PERL_HASH_SEED_DEBUG>.

=item B<bucket_info>

Return a set of basic information about a hash.

    my ($keys, $buckets, $used, @length_counts)= bucket_info($hash);

Fields are as follows:

    0: Number of keys in the hash
    1: Number of buckets in the hash
    2: Number of used buckets in the hash
    rest : list of counts, Kth element is the number of buckets
           with K keys in it.

See also bucket_stats() and bucket_array().

=item B<bucket_stats>

Returns a list of statistics about a hash.

 my ($keys, $buckets, $used, $quality, $utilization_ratio,
        $collision_pct, $mean, $stddev, @length_counts)
    = bucket_stats($hashref);

Fields are as follows:

    0: Number of keys in the hash
    1: Number of buckets in the hash
    2: Number of used buckets in the hash
    3: Hash Quality Score
    4: Percent of buckets used
    5: Percent of keys which are in collision
    6: Mean bucket length of occupied buckets
    7: Standard Deviation of bucket lengths of occupied buckets
    rest : list of counts, Kth element is the number of buckets
           with K keys in it.

See also bucket_info() and bucket_array().

Note that Hash Quality Score would be 1 for an ideal hash, numbers
close to and below 1 indicate good hashing, and number significantly
above indicate a poor score. In practice it should be around 0.95 to 1.05.
It is defined as:

 $score= sum( $count[$length] * ($length * ($length + 1) / 2) )
            /
            ( ( $keys / 2 * $buckets ) *
              ( $keys + ( 2 * $buckets ) - 1 ) )

The formula is from the Red Dragon book (reformulated to use the data available)
and is documented at L<http://www.strchr.com/hash_functions>

=item B<bucket_array>

    my $array= bucket_array(\%hash);

Returns a packed representation of the bucket array associated with a hash. Each element
of the array is either an integer K, in which case it represents K empty buckets, or
a reference to another array which contains the keys that are in that bucket.

B<Note that the information returned by bucket_array is sensitive information>:
by knowing it one can directly attack perl's hash function which in turn may allow
one to craft a denial-of-service attack against Perl code, even remotely,
see L<perlsec/"Algorithmic Complexity Attacks"> for more information.
B<Do not disclose the output of this function> to people who don't need to
know it. See also L<perlrun/PERL_HASH_SEED_DEBUG>. This function is provided strictly
for  debugging and diagnostics purposes only, it is hard to imagine a reason why it
would be used in production code.

=cut


sub bucket_stats {
    my ($hash) = @_;
    my ($keys, $buckets, $used, @length_counts) = bucket_info($hash);
    my $sum;
    my $score;
    for (1 .. $#length_counts) {
        $sum += ($length_counts[$_] * $_);
        $score += $length_counts[$_] * ( $_ * ($_ + 1 ) / 2 );
    }
    $score = $score /
             (( $keys / (2 * $buckets )) * ( $keys + ( 2 * $buckets ) - 1 ))
                 if $keys;
    my ($mean, $stddev)= (0, 0);
    if ($used) {
        $mean= $sum / $used;
        $sum= 0;
        $sum += ($length_counts[$_] * (($_-$mean)**2)) for 1 .. $#length_counts;

        $stddev= sqrt($sum/$used);
    }
    return $keys, $buckets, $used, $keys ? ($score, $used/$buckets, ($keys-$used)/$keys, $mean, $stddev, @length_counts) : ();
}

=item B<bucket_stats_formatted>

  print bucket_stats_formatted($hashref);

Return a formatted report of the information returned by bucket_stats().
An example report looks like this:

 Keys: 50 Buckets: 33/64 Quality-Score: 1.01 (Good)
 Utilized Buckets: 51.56% Optimal: 78.12% Keys In Collision: 34.00%
 Chain Length - mean: 1.52 stddev: 0.66
 Buckets 64          [0000000000000000000000000000000111111111111111111122222222222333]
 Len   0 Pct:  48.44 [###############################]
 Len   1 Pct:  29.69 [###################]
 Len   2 Pct:  17.19 [###########]
 Len   3 Pct:   4.69 [###]
 Keys    50          [11111111111111111111111111111111122222222222222333]
 Pos   1 Pct:  66.00 [#################################]
 Pos   2 Pct:  28.00 [##############]
 Pos   3 Pct:   6.00 [###]

The first set of stats gives some summary statistical information,
including the quality score translated into "Good", "Poor" and "Bad",
(score<=1.05, score<=1.2, score>1.2). See the documentation in
bucket_stats() for more details.

The two sets of barcharts give stats and a visual indication of performance
of the hash.

The first gives data on bucket chain lengths and provides insight on how
much work a fetch *miss* will take. In this case we have to inspect every item
in a bucket before we can be sure the item is not in the list. The performance
for an insert is equivalent to this case, as is a delete where the item
is not in the hash.

The second gives data on how many keys are at each depth in the chain, and
gives an idea of how much work a fetch *hit* will take. The performance for
an update or delete of an item in the hash is equivalent to this case.

Note that these statistics are summary only. Actual performance will depend
on real hit/miss ratios accessing the hash. If you are concerned by hit ratios
you are recommended to "oversize" your hash by using something like:

   keys(%hash)= keys(%hash) << $k;

With $k chosen carefully, and likely to be a small number like 1 or 2. In
theory the larger the bucket array the less chance of collision.

=cut


sub _bucket_stats_formatted_bars {
    my ($total, $ary, $start_idx, $title, $row_title)= @_;

    my $return = "";
    my $max_width= $total > 64 ? 64 : $total;
    my $bar_width= $max_width / $total;

    my $str= "";
    if ( @$ary < 10) {
        for my $idx ($start_idx .. $#$ary) {
            $str .= $idx x sprintf("%.0f", ($ary->[$idx] * $bar_width));
        }
    } else {
        $str= "-" x $max_width;
    }
    $return .= sprintf "%-7s         %6d [%s]\n",$title, $total, $str;

    foreach my $idx ($start_idx .. $#$ary) {
        $return .= sprintf "%-.3s %3d %6.2f%% %6d [%s]\n",
            $row_title,
            $idx,
            $ary->[$idx] / $total * 100,
            $ary->[$idx],
            "#" x sprintf("%.0f", ($ary->[$idx] * $bar_width)),
        ;
    }
    return $return;
}

sub bucket_stats_formatted {
    my ($hashref)= @_;
    my ($keys, $buckets, $used, $score, $utilization_ratio, $collision_pct,
        $mean, $stddev, @length_counts) = bucket_stats($hashref);

    my $return= sprintf   "Keys: %d Buckets: %d/%d Quality-Score: %.2f (%s)\n"
                        . "Utilized Buckets: %.2f%% Optimal: %.2f%% Keys In Collision: %.2f%%\n"
                        . "Chain Length - mean: %.2f stddev: %.2f\n",
                $keys, $used, $buckets, $score, $score <= 1.05 ? "Good" : $score < 1.2 ? "Poor" : "Bad",
                $utilization_ratio * 100,
                $keys/$buckets * 100,
                $collision_pct * 100,
                $mean, $stddev;

    my @key_depth;
    $key_depth[$_]= $length_counts[$_] + ( $key_depth[$_+1] || 0 )
        for reverse 1 .. $#length_counts;

    if ($keys) {
        $return .= _bucket_stats_formatted_bars($buckets, \@length_counts, 0, "Buckets", "Len");
        $return .= _bucket_stats_formatted_bars($keys, \@key_depth, 1, "Keys", "Pos");
    }
    return $return
}

=item B<hv_store>

  my $sv = 0;
  hv_store(%hash,$key,$sv) or die "Failed to alias!";
  $hash{$key} = 1;
  print $sv; # prints 1

Stores an alias to a variable in a hash instead of copying the value.

=item B<hash_traversal_mask>

As of Perl 5.18 every hash has its own hash traversal order, and this order
changes every time a new element is inserted into the hash. This functionality
is provided by maintaining an unsigned integer mask (U32) which is xor'ed
with the actual bucket id during a traversal of the hash buckets using keys(),
values() or each().

You can use this subroutine to get and set the traversal mask for a specific
hash. Setting the mask ensures that a given hash will produce the same key
order. B<Note> that this does B<not> guarantee that B<two> hashes will produce
the same key order for the same hash seed and traversal mask, items that
collide into one bucket may have different orders regardless of this setting.

=item B<bucket_ratio>

This function behaves the same way that scalar(%hash) behaved prior to
Perl 5.25. Specifically if the hash is tied, then it calls the SCALAR tied
hash method, if untied then if the hash is empty it return 0, otherwise it
returns a string containing the number of used buckets in the hash,
followed by a slash, followed by the total number of buckets in the hash.

    my %hash=("foo"=>1);
    print Hash::Util::bucket_ratio(%hash); # prints "1/8"

=item B<used_buckets>

This function returns the count of used buckets in the hash. It is expensive
to calculate and the value is NOT cached, so avoid use of this function
in production code.

=item B<num_buckets>

This function returns the total number of buckets the hash holds, or would
hold if the array were created. (When a hash is freshly created the array
may not be allocated even though this value will be non-zero.)

=back

=head2 Operating on references to hashes.

Most subroutines documented in this module have equivalent versions
that operate on references to hashes instead of native hashes.
The following is a list of these subs. They are identical except
in name and in that instead of taking a %hash they take a $hashref,
and additionally are not prototyped.

=over 4

=item lock_ref_keys

=item unlock_ref_keys

=item lock_ref_keys_plus

=item lock_ref_value

=item unlock_ref_value

=item lock_hashref

=item unlock_hashref

=item lock_hashref_recurse

=item unlock_hashref_recurse

=item hash_ref_unlocked

=item legal_ref_keys

=item hidden_ref_keys

=back

=head1 CAVEATS

Note that the trapping of the restricted operations is not atomic:
for example

    eval { %hash = (illegal_key => 1) }

leaves the C<%hash> empty rather than with its original contents.

=head1 BUGS

The interface exposed by this module is very close to the current
implementation of restricted hashes. Over time it is expected that
this behavior will be extended and the interface abstracted further.

=head1 AUTHOR

Michael G Schwern <schwern@pobox.com> on top of code by Nick
Ing-Simmons and Jeffrey Friedl.

hv_store() is from Array::RefElem, Copyright 2000 Gisle Aas.

Additional code by Yves Orton.

Description of C<hash_value($string, $seed)>
by Christopher Yeleighton <ne01026@shark.2a.pl>

=head1 SEE ALSO

L<Scalar::Util>, L<List::Util> and L<perlsec/"Algorithmic Complexity Attacks">.

L<Hash::Util::FieldHash>.

=cut

1;
