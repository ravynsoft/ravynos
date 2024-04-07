#!./perl

# Verify round trip of translations from the native character set to unicode
# and back work.  If this is wrong, nothing will be reliable.

print "1..257\n";   # 0-255 plus one beyond

for my $i (0 .. 255) {
    my $uni = utf8::native_to_unicode($i);
    if ($uni < 0 || $uni >= 256) {
        print "not ";
    }
    elsif (utf8::unicode_to_native(utf8::native_to_unicode($i)) != $i) {
        print "not ";
    }
    print "ok ";
    print $i + 1 . " - native_to_unicode $i";
    print "\n";
}

# Choose a largish number that might cause a seg fault if inappropriate array
# lookup
if (utf8::unicode_to_native(utf8::native_to_unicode(100000)) != 100000) {
    print "not ";
}
print "ok 257 - native_to_unicode of large number\n";
