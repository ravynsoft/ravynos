s/^\(.TH .*\)/.nr SL @SEMAN@\
\1/

/^\.TP 10n$/ {
	N
	/^.TP 10n\nsesh$/ {
		i\
.if \\n(SL \\{\\
	}
}

/^\\fI@sesh_file@\\fR\.$/ {
	a\
.\\}
}
