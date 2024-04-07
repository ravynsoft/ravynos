package MinimalPerfectHash;
use strict;
use warnings;
use Data::Dumper;
use Carp;
use Text::Wrap;
use List::Util qw(shuffle min);

use warnings 'FATAL' => 'all';

# The style of this file is determined by:
#
# perltidy -w -ple -bbb -bbc -bbs -nolq -l=80 -noll -nola -nwls='=' \
#   -isbc -nolc -otr -kis -ci=4 -se -sot -sct -nsbl -pt=2 -fs  \
#   -fsb='##!' -fse='##.'

# Naming conventions
# * The public API, consisting of methods, uses "normal" sub names with
#   no leading underscore.
# * Private subs are prefixed with a single underscore.
# * Private methods are prefixed with two underscores. (There is only
#   one at the time of writing this comment)

use constant {
    FNV32_PRIME => 16777619,
    U8_MAX      => 0xFF,
    U16_MAX     => 0xFFFF,
    U32_MAX     => 0xFFFFFFFF,
    INF         => 1e9,
};

our $DEBUG= $ENV{DEBUG} || 0;    # our so we can use local on it
my $RSHIFT= 8;
my $MASK= U32_MAX;
my $MAX_SEED2= U16_MAX;          # currently the same, but not necessarily.
my $IS_32BIT= !eval { pack "Q", 1 };

sub new {
    my ($class, %self)= @_;

    my $source_hash= $self{source_hash}
        or die "'source_hash' is a required parameter in $class->new()\n";

    my $length_all_keys= 0;
    $length_all_keys += length($_) for keys %$source_hash;
    $self{length_all_keys}= $length_all_keys;

    $self{max_attempts} ||= 16;    # pick a number, any number...

    $self{base_name} ||= "mph";
    my $base_name= $self{base_name};

    $self{prefix} ||= uc($base_name);

    $self{h_file}      ||= $base_name . "_algo.h";
    $self{c_file}      ||= $base_name . "_test.c";
    $self{t_file}      ||= $base_name . "_test.pl";
    $self{blob_name}   ||= $base_name . "_blob";
    $self{struct_name} ||= $base_name . "_struct";
    $self{table_name}  ||= $base_name . "_table";
    $self{match_name}  ||= $base_name . "_match";

    my $split_strategy;
    $self{simple_split} //= 0;
    if ($self{simple_split}) {
        $self{split_strategy}= "simple";
        $self{randomize_squeeze}= 0;
    }
    else {
        $self{split_strategy}= "squeeze";
        $self{randomize_squeeze} //= 1;
    }
    if ($self{randomize_squeeze}) {
        $self{max_same_in_squeeze} //= 5;
        if (defined $self{srand_seed_was}) {
            $self{srand_seed}= delete $self{srand_seed_was};
        }
        elsif (!defined $self{srand_seed}) {
            $self{srand_seed}= srand();
        }
        else {
            srand($self{srand_seed});
        }
        print "SRAND_SEED= $self{srand_seed}\n" if $DEBUG;
    }
    else {
        $self{max_same}= 3;
        delete $self{srand_seed};
    }
    return bless \%self, $class;
}

# The basic idea is that you have a two level structure, and effectively
# hash the key twice.
#
# The first hash finds a bucket in the array which contains a seed which
# is used for the second hash, which then leads to a bucket with key
# data which is compared against to determine if the key is a match.
#
# If the first hash finds no seed, then the key cannot match.
#
# In our case we cheat a bit, and hash the key only once, but use the
# low bits for the first lookup and the high-bits for the second.
#
# So for instance:
#
#           h= (h >> RSHIFT) ^ s;
#
# is how the second hash is computed. We right shift the original hash
# value  and then xor in the seed2, which will be non-zero.
#
# That then gives us the bucket which contains the key data we need to
# match for a valid key.

sub _fnv1a_32 {
    my ($key, $seed)= @_;
    use integer;

    my $hash= 0 + $seed;
    foreach my $char (split //, $key) {
        $hash= $hash ^ ord($char);

        # the & U32_MAX is to simulate 32 bit ints on a 64 bit integer Perl.
        $hash= ($hash * FNV32_PRIME) & U32_MAX;
    }

    # The hash can end up negative on 32 bit Perls due to use integer being
    # in scope. This is equivalent to casting it to an U32.
    $hash= unpack "V", pack "l", $hash
        if $IS_32BIT;

    return $hash;
}

sub build_perfect_hash {
    my ($self)= @_;

    my $source_hash= $self->{source_hash};
    my $max_attempts= $self->{max_attempts};

    my $n= 0 + keys %$source_hash;
    print "Building a minimal perfect hash from $n keys.\n"
        if $DEBUG;
    my $seed1= unpack("N", "Perl") - 1;

    TRY:
    for (my $attempt= 1 ; $attempt < $max_attempts ; $attempt++) {
        my ($hash_to_key, $key_to_hash, $key_buckets);
        SEED1:
        for ($seed1++ ; 1 ; $seed1++) {
            print "Trying seed $seed1\n"
                if $DEBUG;
            my %hash_to_key;
            my %key_to_hash;
            my %key_buckets;
            my %shifted;
            foreach my $key (sort keys %$source_hash) {
                my $h= _fnv1a_32($key, $seed1);
                next SEED1 if exists $hash_to_key{$h};
                next SEED1 if $shifted{ ($h >> $RSHIFT) & $MASK }++;
                $hash_to_key{$h}= $key;
                $key_to_hash{$key}= $h;
                push @{ $key_buckets{ $h % $n } }, $key;
            }
            $hash_to_key= \%hash_to_key;
            $key_to_hash= \%key_to_hash;
            $key_buckets= \%key_buckets;
            last SEED1;
        }
        my $second_level=
            _build_mph_level2($hash_to_key, $key_to_hash, $key_buckets);
        if ($second_level) {
            $self->{seed1}= $seed1;
            $self->{second_level}= $second_level;
            return $seed1, $second_level;
        }
    }
    die sprintf "After %d attempts failed to construct a minimal perfect "
        . "hash with %d keys.\nWe are using fnv32(), perhaps this "
        . "hash function isn't good enough?\n",
        $max_attempts, $n;
}

sub _build_mph_level2 {
    my ($hash_to_key, $key_to_hash, $key_buckets)= @_;

    my $n= 0 + keys %$key_to_hash;

    # Loop over the key_buckets, processing the buckets with the most
    # items in them first, and the ones with the least items in them last.
    # This maximizes the chance we can find a $seed2 that "disambiguates"
    # the items that collide in a single bucket.
    #
    # With a decent hash function we will have a typical long tail
    # distribution of items per bucket, with relatively few buckets with
    # the most collisions in them, and the vast majority of buckets
    # having no collisions. By processing the ones with the most items
    # in them first the "easy" cases don't get in the way of finding a
    # solution for the hard cases. The buckets can be divided into three
    # levels of difficulty to solve "hard", "medium" and "trivial".
    #
    # * Hard buckets have more than one item in them.
    # * Medium buckets have one item whose hash is above $MAX_SEED2.
    # * Trivial buckets have one item whose hash is not above $MAX_SEED2.
    #
    # Each type of bucket uses a different algorithm to solve. Note that
    # a "classical" two level hash would only have "hard" and "trivial"
    # buckets, but since we support having a larger hash value than we
    # allow for a $seed2 we have three.

    my @first_level;
    my @second_level;
    my @singles_high;
    my @singles_low;

    print "Finding mappings for buckets with collisions.\n"
        if $DEBUG;

    FIRST_IDX:
    foreach my $first_idx (
        sort {
            @{ $key_buckets->{$b} } <=> @{ $key_buckets->{$a} }
                || $a <=> $b
        } keys %$key_buckets
        )
    {
        my $keys= $key_buckets->{$first_idx};
        if (@$keys == 1) {

            # buckets with a single item in them can use a simpler
            # and faster algorithm to find a bucket than those with
            # buckets with more than one item.

            # however keys whose $hash2 is above $MAX_SEED2 need to be
            # processed first, and will use one strategy, while the rest
            # of the singletons should be processed last, and can use
            # an even simpler and more efficient strategy.
            my $key= $keys->[0];
            my $hash2= ($key_to_hash->{$key} >> $RSHIFT) & $MASK;
            if ($hash2 > $MAX_SEED2) {
                push @singles_high, [ $first_idx, $hash2, $key ];
            }
            else {
                push @singles_low, [ $first_idx, $hash2, $key ];
            }
            next FIRST_IDX;
        }

        # This loop handles items with more than one key in the same
        # bucket. We need to find a $seed2 that causes the operation
        #
        #    ($hash ^ $seed2) % $n
        #
        # to map those keys into different empty buckets. If we cannot
        # find such a $seed2 then we need to recompute everything with a
        # new seed.
        SEED2:
        for (my $seed2= 1 ; $seed2 <= $MAX_SEED2 ; $seed2++) {
            my @idx= map {
                ((($key_to_hash->{$_} >> $RSHIFT) ^ $seed2) & $MASK) % $n
            } @$keys;
            my %seen;
            next SEED2 if grep { $second_level[$_] || $seen{$_}++ } @idx;
            $first_level[$first_idx]= $seed2;
            @second_level[@idx]= map { _make_bucket_info($_) } @$keys;
            next FIRST_IDX;
        }

        # If we get here then we failed to find a $seed2 which results
        # in the colliding items being mapped to different empty buckets.
        # So we have to rehash everything with a different $seed1.
        print "Failed to disambiguate colliding keys. Trying new seed1.\n"
            if $DEBUG;
        return;
    }

    # Now fill in the singletons using a much simpler and faster
    # way to compute the seed2. Since we only have to worry about
    # a single seed, we merely need to fill in all the empty slots
    # and we can always compute a mask that when xor'ed with $base
    # maps to the empty slot.
    print "Finding mappings for buckets with no collisions.\n"
        if $DEBUG;

    # sort @singles_low so that for the simple algorithm we do not end
    # up mapping a 0 hash to the 0 bucket, which would result in a
    # $seed2 of 0. Our logic avoids comparing the key when the $seed2 is
    # 0, so we need to avoid having a seed2 of 0. This rule is not
    # strictly required, but it cuts down on string comparisons at the
    # cost of a relatively cheap numeric comparison. If you change this
    # make sure you update the generated C code.

    ##!
    @singles_low= sort {
        $b->[1] <=> $a->[1] ||    # sort by $hash2
        $a->[0] <=> $b->[0]       # then by $first_idx
    } @singles_low;
    ##.

    my $scan_idx= 0;    # used to find empty buckets for the "simple" case.
    SINGLES:
    foreach my $tuple (@singles_high, @singles_low) {
        my ($first_idx, $hash2, $key)= @$tuple;
        my ($seed2, $idx);
        if ($hash2 > $MAX_SEED2) {

            # The $hash2 is larger than the maximum value of $seed2.
            # This means that we cannot simply map this item into
            # whichever bucket we choose using xor. Instead we loop
            # through the possible $seed2 values checking to see if it
            # results in us landing in an empty bucket, which should be
            # fairly common which means this loop should execute
            # relatively few times. It also minimizes the chance that we
            # cannot find a solution at all.
            for my $i (1 .. $MAX_SEED2) {
                $idx= (($hash2 ^ $i) & $MASK) % $n;
                if (!$second_level[$idx]) {
                    $seed2= $i;
                    last;
                }
            }

            # If we failed to find a solution we need to go back to
            # beginning and try a different key.
            if (!defined $seed2) {
                print "No viable seed2 for singleton. Trying new seed1.\n"
                    if $DEBUG;
                return;
            }
        }
        else {
            # since $hash2 <= $MAX_SEED2 we can trivially map the item
            # to any bucket we choose using xor. So we find the next
            # empty bucket with the loop below, and then map this item
            # into it.
            SCAN:
            while ($second_level[$scan_idx]) {
                $scan_idx++;
            }

            # note that we don't need to mod $n here, as
            #
            #   $hash2 ^ $seed2 == $idx
            #
            # and $idx is already in the interval (0, $n-1)

            $seed2= $hash2 ^ $scan_idx;

            # increment $scan_idx after stashing its old value into $idx
            # as by the end of this iteration of the SINGLES loop we
            # will have filled $second_level[$scan_idx] and we need not
            # check it in the SCAN while loop.
            $idx= $scan_idx++;
        }

        # sanity check $idx.
        die "WTF, \$idx should be less than \$n ($idx vs $n)"
            unless $idx < $n;

        die "Bad seed2 for first_idx: $first_idx." if $seed2 == 0;

        # and finally we are done, we have found the final bucket
        # location for this key.
        $first_level[$first_idx]= $seed2;
        $second_level[$idx]= _make_bucket_info($key);
    }

    # now that we are done we can go through and fill in the idx and
    # seed2 as appropriate. We store idx into the hashes even though it
    # is not stricly necessary as it simplifies some of the code that
    # processes the @second_level bucket info array later.
    foreach my $idx (0 .. $n - 1) {
        $second_level[$idx]{seed2}= $first_level[$idx] || 0;
        $second_level[$idx]{idx}= $idx;
    }

    return \@second_level;
}

sub _make_bucket_info {
    my ($key)= @_;
    return +{
        key   => $key,
        seed2 => undef,    # will be filled in later
        idx   => undef,    # will be filled in later
    };
}

sub _sort_keys_longest_first {
    my ($hash)= shift;
    my @keys= sort { length($b) <=> length($a) || $a cmp $b } keys %$hash;
    return \@keys;
}

# This sub constructs a blob of characters which can be used to
# reconstruct the keys of the $hash that is passed in to it, possibly
# and likely by splitting the keys into two parts, a prefix and a
# suffix. This allows prefixes and suffixes to be reused for more than
# one original key.
#
# It returns a string that contains every prefix and suffix chosen, and
# a hash that contains each key in the argument $hash with each value
# being the position where it is split, using the length of the key to
# indicate it need not be split at all.
#
# If $preprocess is false the process starts with an empty buffer and
# populates it as it adds each new key, if $preprocess is true then it
# tries to split each key at the '=' sign which is often present in
# Unicode property names and composes the initial buffer from these
# fragments.
#
# It performs multiple passes trying to find the ideal split point to
# produce a minimal buffer, returning the smallest buffer it can.
sub _build_split_words_simple {
    my ($hash, $length_all_keys, $preprocess)= @_;
    my %appended;
    my $blob= "";
    if ($preprocess) {
        my %parts;
        foreach my $key (@{ _sort_keys_longest_first($hash) }) {
            my ($prefix, $suffix);
            if ($key =~ /^([^=]+=)([^=]+)\z/) {
                ($prefix, $suffix)= ($1, $2);
                $parts{$suffix}++;

                #$parts{$prefix}++;
            }
            else {
                $prefix= $key;
                $parts{$prefix}++;
            }

        }
        foreach my $part (@{ _sort_keys_longest_first(\%parts) }) {
            $blob .= $part;
        }
        printf "Using preprocessing, initial blob size is %d chars.\n",
            length($blob)
            if $DEBUG;
    }
    else {
        print "No preprocessing, starting with an empty blob.\n"
            if $DEBUG;
    }
    my ($res, $old_res, $added, $passes);

    REDO:
    $res= {};
    $added= 0;
    $passes++;

    KEY:
    foreach my $key (@{ _sort_keys_longest_first($hash) }) {
        next if exists $res->{$key};
        if (index($blob, $key) >= 0) {
            my $idx= length($key);
            if ($DEBUG > 1 and $old_res and $old_res->{$key} != $idx) {
                print "changing: $key => $old_res->{$key} : $idx\n";
            }
            $res->{$key}= $idx;
            next KEY;
        }
        my $best= length($key);
        my $append= $key;
        my $best_prefix= $key;
        my $best_suffix= "";
        my $min= 1;
        foreach my $idx (reverse $min .. length($key) - 1) {
            my $prefix= substr($key, 0, $idx);
            my $suffix= substr($key, $idx);
            my $i1= index($blob, $prefix) >= 0;
            my $i2= index($blob, $suffix) >= 0;
            if ($i1 and $i2) {
                if ($DEBUG > 1 and $old_res and $old_res->{$key} != $idx) {
                    print "changing: $key => $old_res->{$key} : $idx\n";
                }
                $res->{$key}= $idx;
                $appended{$prefix}++;
                $appended{$suffix}++;
                next KEY;
            }
            elsif ($i1) {
                if (length $suffix <= length $append) {
                    $best= $idx;
                    $append= $suffix;
                    $best_prefix= $prefix;
                    $best_suffix= $suffix;
                }
            }
            elsif ($i2) {
                if (length $prefix <= length $append) {
                    $best= $idx;
                    $append= $prefix;
                    $best_prefix= $prefix;
                    $best_suffix= $suffix;
                }
            }
        }
        if ($DEBUG > 1 and $old_res and $old_res->{$key} != $best) {
            print "changing: $key => $old_res->{$key} : $best\n";
        }

        $res->{$key}= $best;
        $blob .= $append;
        $added += length($append);
        $appended{$best_prefix}++;
        $appended{$best_suffix}++;
    }
    if ($added) {
        if ($added < length $blob) {
            printf "Appended %d chars. Blob is %d chars long.\n",
                $added, length($blob)
                if $DEBUG;
        }
        else {
            printf "Blob is %d chars long.\n", $added
                if $DEBUG;
        }
    }
    elsif ($passes > 1) {
        print "Blob needed no changes.\n"
            if $DEBUG;
    }
    my $new_blob= "";
    foreach my $part (@{ _sort_keys_longest_first(\%appended) }) {
        $new_blob .= $part unless index($new_blob, $part) >= 0;
    }
    if (length($new_blob) < length($blob)) {
        printf "Uncorrected new blob length of %d chars is smaller.\n"
            . "  Correcting new blob...%s",
            length($new_blob), $DEBUG > 1 ? "\n" : " "
            if $DEBUG;
        $blob= $new_blob;
        $old_res= $res;
        %appended= ();
        goto REDO;
    }
    else {
        printf "After %d passes final blob length is %d chars.\n"
            . "This is %.2f%% of the raw key length of %d chars.\n\n",
            $passes, length($blob), 100 * length($blob) / $length_all_keys,
            $length_all_keys
            if $DEBUG;
    }

    # sanity check
    die sprintf "not same size? %d != %d", 0 + keys %$res, 0 + keys %$hash
        unless keys %$res == keys %$hash;
    return ($blob, $res, $length_all_keys);
}

# Find all the positions where $word can be found in $$buf_ref,
# including overlapping positions. The data is cached into the
# $offsets_hash. Used by the _squeeze algorithm.
sub _get_offsets {
    my ($offsets_hash, $buf_ref, $word)= @_;
    return $offsets_hash->{$word}
        if defined $offsets_hash->{$word};

    my @offsets;
    my $from= 0;

    while (1) {
        my $i= index($$buf_ref, $word, $from);
        last if $i == -1;
        push @offsets, $i;
        $from= $i + 1;
    }

    $offsets_hash->{$word}= \@offsets;
    return \@offsets;
}

# Increments the popularity data for the characters at
# $ofs .. $ofs + $len - 1 by $diff. Used by the _squeeze algorithm
sub _inc_popularity {
    my ($popularity, $ofs, $len, $diff)= @_;
    for my $idx ($ofs .. $ofs + $len - 1) {
        $popularity->[$idx] += $diff;
    }
}

# Returns a summary hash about the popularity of the characters
# $ofs .. $ofs + $len - 1. Used by the _squeeze algorithm
sub _get_popularity {
    my ($popularity, $ofs, $len)= @_;
    my $res= {
        reused_digits => 0,
        popularity    => 0,
    };
    my $min_pop= undef;
    for my $idx ($ofs .. $ofs + $len - 1) {
        if ($popularity->[$idx] >= INF) {
            $res->{reused_digits}++;
        }
        else {
            my $pop= $popularity->[$idx];
            if (!defined $min_pop || $pop < $min_pop) {
                $min_pop= $pop;
            }
        }
    }
    $res->{popularity}= $min_pop // 0;
    return $res;
}

# Merge the popularity data produced by _get_popularity() for the prefix
# and suffix of a word together. Used by the _squeeze algorithm
sub _merge_score {
    my ($s1, $s2)= @_;
    return +{
        reused_digits => $s1->{reused_digits} + $s2->{reused_digits},
        popularity    => min($s1->{popularity}, $s2->{popularity}),
    };
}

# Initialize the popularity and offsets data for a word.
# Used by the _squeeze algorithm
sub _init_popularity {
    my ($offsets_hash, $popularity, $buf_ref, $word, $diff)= @_;
    my $offsets= _get_offsets($offsets_hash, $buf_ref, $word);
    my $len= length $word;
    for my $ofs (@$offsets) {
        for my $idx ($ofs .. $ofs + $len - 1) {
            $popularity->[$idx] += $diff;
        }
    }
}

# Compare the popularity data for two possible candidates
# for solving a given word. Used by the _squeeze algorithm
sub _compare_score {
    my ($s1, $s2)= @_;
    if ($s1->{reused_digits} != $s2->{reused_digits}) {
        return $s1->{reused_digits} <=> $s2->{reused_digits};
    }
    return $s1->{popularity} <=> $s2->{popularity};
}

# Find the most popular offset for a word in $$buf_ref.
# Used by the _squeeze algorithm
sub _most_popular_offset {
    my ($offsets_hash, $popularity, $buf_ref, $word)= @_;
    my $best_score= {
        reused_digits => -1,
        popularity    => -1,
    };
    my $best_pos= -1;
    my $offsets_ary= _get_offsets($offsets_hash, $buf_ref, $word);
    my $wlen= length $word;
    for my $i (@$offsets_ary) {
        my $score= _get_popularity($popularity, $i, $wlen);
        if (_compare_score($score, $best_score) > 0) {
            $best_score= $score;
            $best_pos= $i;
            if ($best_score->{reused_digits} == $wlen) {
                last;
            }
        }
    }
    return +{
        position => $best_pos,
        score    => $best_score,
    };
}

# The _squeeze algorithm. Attempt to squeeze out unused characters from
# a buffer of split words. If there are multiple places where a given
# prefix or suffix can be found and the overall split decisions can be
# reorganized so some of them are never used it removes the ones that
# are not used.
sub _squeeze {
    my ($words, $word_count, $splits, $buf_ref)= @_;
    print "Squeezing...\n" if $DEBUG;
    my %offsets_hash;
    my %split_points;
    my $n= length $$buf_ref;
    my @popularity= 0 x $n;

    for my $word (sort keys %$word_count) {
        my $count= $word_count->{$word};
        _init_popularity(\%offsets_hash, \@popularity, $buf_ref, $word,
            $count / length($word));
    }

    WORD:
    for my $word (@$words) {
        my $best_pos1= -1;
        my $best_pos2= -1;
        my $best_score= {
            reused_digits => -1,
            popularity    => -1,
        };
        my $best_split;

        my $cand=
            _most_popular_offset(\%offsets_hash, \@popularity, $buf_ref, $word);
        if ($cand->{position} != -1) {
            my $cand_score= $cand->{score};
            if ($cand_score->{reused_digits} == length($word)) {
                $split_points{$word}= 0;
                next WORD;
            }
            elsif (_compare_score($cand_score, $best_score) > 0) {
                $best_score= $cand_score;
                $best_pos1= $cand->{position};
                $best_pos2= -1;
                $best_split= undef;
            }
        }

        for my $split (@{ $splits->{$word} }) {
            my $cand2=
                _most_popular_offset(\%offsets_hash, \@popularity, $buf_ref,
                $split->{w2});
            next if $cand2->{position} == -1;

            my $cand1=
                _most_popular_offset(\%offsets_hash, \@popularity, $buf_ref,
                $split->{w1});
            next if $cand1->{position} == -1;

            my $cand_score= _merge_score($cand1->{score}, $cand2->{score});

            if ($cand_score->{reused_digits} == length($word)) {
                $split_points{$word}= $split->{split_point};
                next WORD;
            }
            if (_compare_score($cand_score, $best_score) > 0) {
                $best_score= $cand_score;
                $best_pos1= $cand1->{position};
                $best_pos2= $cand2->{position};
                $best_split= $split;
            }
        }

        # apply high pop to used characters of the champion
        if (defined $best_split) {
            _inc_popularity(\@popularity, $best_pos1,
                length($best_split->{w1}), INF);
            _inc_popularity(\@popularity, $best_pos2,
                length($best_split->{w2}), INF);
            $split_points{$word}= $best_split->{split_point};
        }
        else {
            _inc_popularity(\@popularity, $best_pos1, length($word), INF);
            $split_points{$word}= 0;
        }
    }

    my $res= "";
    my @chars= split '', $$buf_ref;
    for my $i (0 .. $n - 1) {
        if ($popularity[$i] >= INF) {
            $res .= $chars[$i];
        }
    }
    printf "%d -> %d\n", $n, length($res) if $DEBUG;

    # This algorithm chooses to "split" full strings at 0, so that the
    # prefix is empty and the suffix contains the full key, but the
    # minimal perfect hash logic wants it the other way around, as we do
    # the prefix check first. so we correct it at the end here.
    $split_points{$_} ||= length($_) for keys %split_points;

    return ($res, \%split_points);
}

# compute an initial covering buffer for a set of words,
# including split data.
sub _initial_covering_buf {
    my ($words, $splits)= @_;
    my $res= "";
    WORD:
    for my $word (@$words) {
        if (index($res, $word) != -1) {
            next WORD;
        }
        else {
            for my $split (@{ $splits->{$word} }) {
                if (   index($res, $split->{w1}) != -1
                    && index($res, $split->{w2}) != -1)
                {
                    next WORD;
                }
            }
        }
        $res .= $word;
    }
    return $res;
}

sub build_split_words_squeeze {
    my ($self)= @_;
    # Thanks to Ilya Sashcheka for this algorithm

    my $hash= $self->{source_hash};
    my $length_all_keys= $self->{length_all_keys};
    my $randomize= $self->{randomize_squeeze};
    my $max_same= $self->{max_same_in_squeeze};

    my @words= sort keys %$hash;
    my %splits;
    my $split_points;

    for my $word (@words) {
        my $word_splits= [];
        my $wlen= length $word;
        for my $i (1 .. $wlen - 1) {
            ##!
            push @$word_splits,
                +{
                    w1          => substr($word, 0, $i),
                    w2          => substr($word, $i),
                    split_point => $i,
                };
            ##.
        }
        $splits{$word}= $word_splits;
    }

    my %word_count;
    for my $word (@words) {
        $word_count{$word}++;
        for my $split (@{ $splits{$word} }) {
            $word_count{ $split->{w1} }++;
            $word_count{ $split->{w2} }++;
        }
    }

    @words= sort { length($a) <=> length($b) || $a cmp $b } @words;
    my $buf= _initial_covering_buf(\@words, \%splits);

    printf "Pre squeeze buffer: %s\n", $buf        if $DEBUG > 1;
    printf "Pre squeeze length: %d\n", length $buf if $DEBUG;

    my $same= 0;
    my $counter= 0;
    my $reverse_under= 2;
    while ($same < $max_same) {
        my ($new_buf, $new_split_points)=
            _squeeze(\@words, \%word_count, \%splits, \$buf);
        if (!$split_points or length($new_buf) < length($buf)) {
            $buf= $new_buf;
            $split_points= $new_split_points;
            $same= 0;
        }
        else {
            if ($same < $reverse_under or !$randomize) {
                print "reversing words....\n" if $DEBUG;
                @words= reverse @words;
            }
            else {
                print "shuffling words....\n" if $DEBUG;
                @words= shuffle @words;
                $reverse_under= 1;
            }
            $same++;
        }
    }

    printf "Final length: %d\n", length($buf) if $DEBUG;

    $self->{blob}= $buf;
    $self->{split_points}= $split_points;

    return $buf, $split_points;
}

sub build_split_words_simple {
    my ($self)= @_;

    my $hash= $self->{source_hash};
    my $length_all_keys= $self->{length_all_keys};

    my ($blob, $split_points)=
        _build_split_words_simple($hash, $length_all_keys, 0);

    my ($blob2, $split_points2)=
        _build_split_words_simple($hash, $length_all_keys, 1);

    if (length($blob) > length($blob2)) {
        printf "Using preprocess-smart blob. Length is %d chars. (vs %d)\n",
            length $blob2, length $blob
            if $DEBUG;
        $blob= $blob2;
        $split_points= $split_points2;
    }
    else {
        printf "Using greedy-smart blob. Length is %d chars. (vs %d)\n",
            length $blob, length $blob2
            if $DEBUG;
    }
    $self->{blob}= $blob;
    $self->{split_points}= $split_points;

    return $blob, $split_points;
}

sub build_split_words {
    my ($self)= @_;

    # The _simple algorithm does not compress nearly as well as the
    # _squeeze algorithm, although it uses less memory and will likely
    # be faster, especially if randomization is enabled. The default
    # is to use _squeeze as our hash is not that large (~8k keys).
    my ($buf, $split_words);
    if ($self->{simple_split}) {
        ($buf, $split_words)= $self->build_split_words_simple();
    }
    else {
        ($buf, $split_words)= $self->build_split_words_squeeze();
    }
    foreach my $key (sort keys %$split_words) {
        my $point= $split_words->{$key};
        my $prefix= substr($key, 0, $point);
        my $suffix= substr($key, $point);
        if (index($buf, $prefix) < 0) {
            die "Failed to find prefix '$prefix' for '$key'";
        }
        if (length $suffix and index($buf, $suffix) < 0) {
            die "Failed to find suffix '$suffix' for '$key'";
        }
    }
    return ($buf, $split_words);
}

sub blob_as_code {
    my ($self)= @_;
    my $blob= $self->{blob};
    my $blob_name= $self->{blob_name};

    # output the blob as C code.
    my @code= (sprintf "STATIC const unsigned char %s[] =\n", $blob_name);
    my $blob_len= length $blob;
    while (length($blob)) {
        push @code, sprintf qq(    "%s"), substr($blob, 0, 65, "");
        push @code, length $blob ? "\n" : ";\n";
    }
    push @code, "/* $blob_name length: $blob_len */\n";
    return $self->{blob_as_code}= join "", @code;
}

sub print_includes {
    my ($self, $ofh)= @_;
    print $ofh "#include <stdio.h>\n";
    print $ofh "#include <string.h>\n";
    print $ofh "#include <stdint.h>\n";
    print $ofh "\n";
}

sub print_defines {
    my ($self, $ofh)= @_;
    my $defines= $self->{defines_hash};

    my $key_len;
    foreach my $def (keys %$defines) {
        $key_len //= length $def;
        $key_len= length $def if $key_len < length $def;
    }
    foreach my $def (sort keys %$defines) {
        printf $ofh "#define %*s %5d\n", -$key_len, $def, $defines->{$def};
    }
    print $ofh "\n";
}

sub build_array_of_struct {
    my ($self)= @_;
    my $second_level= $self->{second_level};
    my $blob= $self->{blob};

    my %defines;
    my %tests;
    my @rows;
    foreach my $row (@$second_level) {
        if (!defined $row->{idx} or !defined $row->{value}) {
            die "panic: No idx or value key in row data:", Dumper($row);
        }
        $defines{ $row->{value} }= $row->{idx} + 1;
        $tests{ $row->{key} }= $defines{ $row->{value} };
        ##!
        my @u16= (
            $row->{seed2},
            index($blob, $row->{prefix}),
            index($blob, $row->{suffix}),
        );
        $_ > U16_MAX and die "panic: value exceeds range of U16"
            for @u16;
        my @u8= (
            length($row->{prefix}),
            length($row->{suffix}),
        );
        $_ > U8_MAX and die "panic: value exceeds range of U8"
            for @u8;
        push @rows, sprintf "  { %5d, %5d, %5d, %3d, %3d, %s }   /* %s%s */",
            @u16, @u8, $row->{value}, $row->{prefix}, $row->{suffix};
        ##.
    }
    $self->{rows_array}= \@rows;
    $self->{defines_hash}= \%defines;
    $self->{tests_hash}= \%tests;
    return \@rows, \%defines, \%tests;
}

sub make_algo {
    my ($self)= @_;

    my (
        $second_level, $seed1,     $length_all_keys, $blob,
        $rows_array,   $blob_name, $struct_name,     $table_name,
        $match_name,   $prefix,    $split_strategy,  $srand_seed,
        )
        = @{$self}{ qw(
            second_level   seed1       length_all_keys   blob
            rows_array     blob_name   struct_name       table_name
            match_name     prefix      split_strategy    srand_seed
        ) };

    my $n= 0 + @$second_level;
    my $data_size= $n * 8 + length $blob;

    my @code= "#define ${prefix}_VALt I16\n\n";
    push @code, "/*\n";
    push @code, sprintf "generator script: %s\n", $0;
    push @code, sprintf "split strategy: %s\n",   $split_strategy;
    push @code, sprintf "srand: %d\n", $srand_seed
        if defined $srand_seed;
    push @code, sprintf "rows: %s\n",                $n;
    push @code, sprintf "seed: %s\n",                $seed1;
    push @code, sprintf "full length of keys: %d\n", $length_all_keys;
    push @code, sprintf "blob length: %d\n",         length $blob;
    push @code, sprintf "ref length: %d\n",          0 + @$second_level * 8;
    push @code, sprintf "data size: %d (%%%.2f)\n", $data_size,
        ($data_size / $length_all_keys) * 100;
    push @code, "*/\n\n";

    push @code, $self->blob_as_code();
    push @code, <<"EOF_CODE";

struct $struct_name {
    U16 seed2;
    U16 pfx;
    U16 sfx;
    U8  pfx_len;
    U8  sfx_len;
    ${prefix}_VALt value;
};

EOF_CODE

    push @code, "#define ${prefix}_RSHIFT $RSHIFT\n";
    push @code, "#define ${prefix}_BUCKETS $n\n\n";
    push @code, sprintf "STATIC const U32 ${prefix}_SEED1 = 0x%08x;\n", $seed1;
    push @code, sprintf "STATIC const U32 ${prefix}_FNV32_PRIME = 0x%08x;\n\n",
        FNV32_PRIME;

    push @code, "/* The comments give the input key for the row it is in */\n";
    push @code,
        "STATIC const struct $struct_name $table_name\[${prefix}_BUCKETS] = {\n",
        join(",\n", @$rows_array) . "\n};\n\n";
    push @code, <<"EOF_CODE";
${prefix}_VALt
$match_name( const unsigned char * const key, const U16 key_len ) {
    const unsigned char * ptr= key;
    const unsigned char * ptr_end= key + key_len;
    U32 h= ${prefix}_SEED1;
    U32 s;
    U32 n;
    /* this is FNV-1a 32bit unrolled. */
    do {
        h ^= NATIVE_TO_LATIN1(*ptr);    /* table collated in Latin1 */
        h *= ${prefix}_FNV32_PRIME;
    } while ( ++ptr < ptr_end );
    n= h % ${prefix}_BUCKETS;
    s = $table_name\[n].seed2;
    if (s) {
        h= (h >> ${prefix}_RSHIFT) ^ s;
        n = h % ${prefix}_BUCKETS;
        if (
            ( $table_name\[n].pfx_len + $table_name\[n].sfx_len == key_len ) &&
            ( memcmp($blob_name + $table_name\[n].pfx, key, $table_name\[n].pfx_len) == 0 ) &&
            ( !$table_name\[n].sfx_len || memcmp($blob_name + $table_name\[n].sfx,
                key + $table_name\[n].pfx_len, $table_name\[n].sfx_len) == 0 )
        ) {
            return $table_name\[n].value;
        }
    }
    return 0;
}
EOF_CODE

    return $self->{algo_code}= join "", @code;
}

sub __ofh {
    my ($self, $to, $default_key)= @_;

    $to //= $self->{$default_key};

    my $ofh;
    if (ref $to) {
        $ofh= $to;
    }
    else {
        open $ofh, ">", $to
            or die "Failed to open '$to': $!";
    }
    return $ofh;
}

sub print_algo {
    my ($self, $to)= @_;

    my $ofh= $self->__ofh($to, "h_file");

    my $code= $self->make_algo();
    print $to $code;
}

sub print_main {
    my ($self, $ofh)= @_;
    my ($h_file, $match_name, $prefix)= @{$self}{qw(h_file match_name prefix)};
    print $ofh <<"EOF_CODE";
#include "$h_file"

int main(int argc, char *argv[]){
    int i;
    for (i=1; i<argc; i++) {
        unsigned char *key = (unsigned char *)argv[i];
        int key_len = strlen(argv[i]);
        printf("key: %s got: %d\\n", key, $match_name((unsigned char *)key,key_len));
    }
    return 0;
}
EOF_CODE
}

# output the test Perl code.
sub print_tests {
    my ($self, $to)= @_;
    my $tests_hash= $self->{tests_hash};

    my $ofh= $self->__ofh($to, "t_file");

    my $num_tests= 2 + keys %$tests_hash;
    print $ofh
        "use strict;\nuse warnings;\nuse Test::More tests => $num_tests;\nmy \@res;";
    my $bytes= 0;
    my @tests= sort keys %$tests_hash;
    print $ofh
        "\@res=`./mph_test '$tests[0]/should-not-match' 'should-not-match/$tests[0]'`;\n";
    print $ofh "ok( \$res[0] =~ /got: 0/,'proper prefix does not match');\n";
    print $ofh "ok( \$res[1] =~ /got: 0/,'proper suffix does not match');\n";

    while (@tests) {
        my @batch= splice @tests, 0, 10;
        my $batch_args= join " ", map { "'$_'" } @batch;
        print $ofh "\@res=`./mph_test $batch_args`;\n";
        foreach my $i (0 .. $#batch) {
            my $key= $batch[$i];
            my $want= $tests_hash->{$key};
            print $ofh
                "ok(\$res[$i]=~/got: (\\d+)/ && \$1 == $want, '$key');\n";
        }
    }
}

sub print_test_binary {
    my ($self, $to)= @_;

    my $ofh= $self->__ofh($to, "c_file");

    $self->print_includes($ofh);
    $self->print_defines($ofh);
    $self->print_main($ofh);
}

sub make_mph_with_split_keys {
    my ($self)= @_;

    my $hash= $self->{source_hash};
    my $length_all_keys= $self->{length_all_keys};

    my ($blob, $split_points)= $self->build_split_words();

    my ($seed1, $second_level)= $self->build_perfect_hash();

    # add prefix/suffix data into the bucket info in @$second_level
    foreach my $bucket_info (@$second_level) {
        my $key= $bucket_info->{key};
        my $sp= $split_points->{$key} // die "no split_point data for '$key'\n";

        my ($prefix, $suffix)= unpack "A${sp}A*", $key;
        $bucket_info->{prefix}= $prefix;
        $bucket_info->{suffix}= $suffix;
        $bucket_info->{value}= $hash->{$key};
    }
    my ($rows, $defines, $tests)= $self->build_array_of_struct();
    return 1;
}

sub make_files_split_keys {
    my ($self)= @_;

    $self->make_mph_with_split_keys();
    $self->print_algo();
    $self->print_test_binary();
    $self->print_tests();
}

unless (caller) {
    my %hash;
    {
        no warnings;
        do "../perl/lib/unicore/UCD.pl";
        %hash= %utf8::loose_to_file_of;
    }
    if ($ENV{MERGE_KEYS}) {
        my @keys= keys %hash;
        foreach my $loose (keys %utf8::loose_property_name_of) {
            my $to= $utf8::loose_property_name_of{$loose};
            next if $to eq $loose;
            foreach my $key (@keys) {
                my $copy= $key;
                if ($copy =~ s/^\Q$to\E(=|\z)/$loose$1/) {

                    $hash{$copy}= $key;
                }
            }
        }
    }
    foreach my $key (keys %hash) {
        my $munged= uc($key);
        $munged =~ s/\W/__/g;
        $hash{$key}= $munged;
    }

    my $name= shift @ARGV;
    $name ||= "mph";
    my $obj= __PACKAGE__->new(
        source_hash => \%hash,
        base_name   => $name
    );
    $obj->make_files_split_keys();
}

1;
__END__
