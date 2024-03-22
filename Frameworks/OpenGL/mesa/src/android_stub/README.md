The Android NDK doesn't come with enough of the platform libraries we
need to build Mesa drivers out of tree, so android_stub has stub
versions of those library that aren't installed which we link against,
relying on the real libraries to be present when the Mesa driver is
deployed.
