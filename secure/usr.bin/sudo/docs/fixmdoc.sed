# Replace "0 minutes" with "unlimited"
/^\.Li 0$/ {
	N
	s/^\.Li 0\nminutes\.$/unlimited./
}
