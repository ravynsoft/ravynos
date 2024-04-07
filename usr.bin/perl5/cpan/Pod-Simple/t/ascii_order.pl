# Helper for some of the .t's in this directory

sub native_to_uni($) {  # Convert from platform character set to Unicode
                        # (which is the same as ASCII)
    my $string = shift;

    return $string if ord("A") == 65
                      || $] lt 5.007_003; # Doesn't work on early EBCDIC Perls
    my $output = "";
    for my $i (0 .. length($string) - 1) {
        $output .= chr(utf8::native_to_unicode(ord(substr($string, $i, 1))));
    }
    # Preserve utf8ness of input onto the output, even if it didn't need to be
    # utf8
    utf8::upgrade($output) if utf8::is_utf8($string);

    return $output;
}


sub ascii_order {   # Sort helper.  Causes the order to be the same as ASCII
                    # no matter what the platform's character set is.
    return native_to_uni($a) cmp native_to_uni($b);
}

1
