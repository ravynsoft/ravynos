package Cname;
our $Evil='A';

sub translator {

    # Returns the input as a name, except for these special ones

    my $str = shift;
    if ( $str eq 'EVIL' ) {
        # Returns A first time, AB second, ABC third ... A-ZA the 27th time.
        (my $c=substr("A".$Evil,-1))++;
        my $r=$Evil;
        $Evil.=$c;
        return $r;
    }
    if ( $str eq 'EMPTY-STR') {
       return "";
    }
    if ( $str eq 'NULL') {
        return "\0";
    }
    if ( $str eq 'LONG-STR') {
        return 'A' x 255;
    }
    # Should exceed limit for regex \N bytes in a sequence.  Anyway it will if
    # UCHAR_MAX is 255.
    if ( $str eq 'TOO-LONG-STR') {
       return 'A' x 256;
    }

    return $str;
}

sub import {
    shift;
    $^H{charnames} = \&translator;
}
1;  
