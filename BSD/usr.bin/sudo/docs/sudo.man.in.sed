s/^\(.TH .*\)/.nr SL @SEMAN@\
.nr BA @BAMAN@\
.nr LC @LCMAN@\
.nr PS @PSMAN@\
\1/

s/^\(\[\\fB\\-a\\fR.*\\fItype\\fR\]\) *$/.if \\n(BA \1/
s/^\(\[\\fB\\-c\\fR.*\\fIclass\\fR\]\) *$/.if \\n(LC \1/
s/^\(\[\\fB\\-r\\fR.*\\fIrole\\fR\]\) *$/.if \\n(SL \1/
s/^\(\[\\fB\\-t\\fR.*\\fItype\\fR\]\) *$/.if \\n(SL \1/

/^\.TP 12n$/ {
	N
	/^\.TP 12n\n\\fB\\-a\\fR.*\\fItype\\fR$/,/^\.TP 12n/ {
            /^\.TP 12n/ {
		/^\.TP 12n\n\\fB\\-a\\fR.*\\fItype\\fR$/i\
.if \\n(BA \\{\\
		/^\.TP 12n\n\\fB\\-a\\fR.*\\fItype\\fR$/!i\
.\\}
            }
        }
	/^\.TP 12n\n\\fB\\-c\\fR.*\\fIclass\\fR$/,/^\.TP 12n/ {
            /^\.TP 12n/ {
		/^\.TP 12n\n\\fB\\-c\\fR.*\\fIclass\\fR$/i\
.if \\n(LC \\{\\
		/^\.TP 12n\n\\fB\\-c\\fR.*\\fIclass\\fR$/!i\
.\\}
            }
        }
	/^\.TP 12n\n\\fB\\-r\\fR.*\\fIrole\\fR$/,/^\.TP 12n/ {
            /^\.TP 12n/ {
		/^\.TP 12n\n\\fB\\-r\\fR.*\\fIrole\\fR$/i\
.if \\n(SL \\{\\
		/^\.TP 12n\n\\fB\\-r\\fR.*\\fIrole\\fR$/!i\
.\\}
            }
        }
	/^\.TP 12n\n\\fB\\-t\\fR.*\\fItype\\fR$/,/^\.TP 12n/ {
            /^\.TP 12n/ {
		/^\.TP 12n\n\\fB\\-t\\fR.*\\fItype\\fR$/i\
.if \\n(SL \\{\\
		/^\.TP 12n\n\\fB\\-t\\fR.*\\fItype\\fR$/!i\
.\\}
            }
        }
}

/^\.TP 3n$/ {
	N
	N
	/^.TP 3n\n\\fB\\(bu\\fR\nSELinux role and type$/ {
		i\
.if \\n(SL \\{\\
		a\
.\\}
	}
	/^.TP 3n\n\\fB\\(bu\\fR\nSolaris project$/ {
		i\
.if \\n(PS \\{\\
		a\
.\\}
	}
	/^.TP 3n\n\\fB\\(bu\\fR\nSolaris privileges$/ {
		i\
.if \\n(PS \\{\\
		a\
.\\}
	}
	/^.TP 3n\n\\fB\\(bu\\fR\nBSD$/ {
		N
		i\
.if \\n(LC \\{\\
		a\
.\\}
	}
}
