s/^\(.TH .*\)/.nr SL @SEMAN@\
.nr BA @BAMAN@\
.nr LC @LCMAN@\
.nr PS @PSMAN@\
\1/

/^On$/N
/^On\nBSD$/,/^.*\.$/ {
    /^On\nBSD$/i\
.if \\n(LC \\{\\
    /\.$/a\
.\\}
}

/^\.SS "SELinux_Spec"$/,/^\.SS/ {
    /^\.SS / {
	/^\.SS "SELinux_Spec"$/i\
.if \\n(SL \\{\\
	/^\.SS "SELinux_Spec"$/!i\
.\\}
    }
}

/^\.SS "Solaris_Priv_Spec"$/,/^\.SS/ {
    /^\.SS / {
	/^\.SS "Solaris_Priv_Spec"$/i\
.if \\n(PS \\{\\
	/^\.SS "Solaris_Priv_Spec"$/!i\
.\\}
    }
}

/^Option_Spec ::= / {
    s/^.*$/.ie \\n(SL \\{\\\
.ie \\n(PS Option_Spec ::= (SELinux_Spec | Solaris_Priv_Spec | Date_Spec | Timeout_Spec)\
.el Option_Spec ::= (SELinux_Spec | Date_Spec | Timeout_Spec)\
.\\}\
.el \\{\\\
.ie \\n(PS Option_Spec ::= (Solaris_Priv_Spec | Date_Spec | Timeout_Spec)\
.el Option_Spec ::= (Date_Spec | Timeout_Spec)\
.\\}/
}

/^SELinux_Spec ::=/ {
    i\
.if \\n(SL \\{\\
    N
    a\
.\\}
}

/^Solaris_Priv_Spec ::=/ {
    i\
.if \\n(PS \\{\\
    N
    a\
.\\}
}

/^SELinux roles.*types,/ {
    i\
.if \\n(SL \\{\\
    a\
.\\}
}

/^Solaris privileges sets,/ {
    i\
.if \\n(PS \\{\\
    a\
.\\}
}

/^\.TP 18n$/ {
	N
	/^\.TP 18n\nuse_loginclass$/,/^\.TP 18n/ {
	    /^\.TP 18n/ {
		/^\.TP 18n\nuse_loginclass$/i\
.if \\n(BA \\{\\
		/^\.TP 18n\nuse_loginclass$/!i\
.\\}
	    }
	}
	/^\.TP 18n\nlimitprivs$/,/^\.TP 18n/ {
	    /^\.TP 18n/ {
		/^\.TP 18n\nlimitprivs$/i\
.if \\n(PS \\{\\
		/^\.TP 18n\nlimitprivs$/!i\
.\\}
	    }
	}
	/^\.TP 18n\nprivs$/,/^\.TP 18n/ {
	    /^\.TP 18n/ {
		/^\.TP 18n\nprivs$/i\
.if \\n(PS \\{\\
		/^\.TP 18n\nprivs$/!i\
.\\}
	    }
	}
	/^\.TP 18n\nselinux$/,/^\.TP 18n/ {
	    /^\.TP 18n/ {
		/^\.TP 18n\nselinux$/i\
.if \\n(SL \\{\\
		/^\.TP 18n\nselinux$/!i\
.\\}
	    }
	}
	/^\.TP 18n\nrole$/,/^\.TP 18n/ {
	    /^\.TP 18n/ {
		/^\.TP 18n\nrole$/i\
.if \\n(SL \\{\\
		/^\.TP 18n\nrole$/!i\
.\\}
	    }
	}
	/^\.TP 18n\ntype$/,/^\.TP 18n/ {
	    /^\.TP 18n/ {
		/^\.TP 18n\ntype$/i\
.if \\n(SL \\{\\
		/^\.TP 18n\ntype$/!i\
.\\}
	    }
	}
}

/^\\fRPRIVS\\fR,/ {
    i\
.if \\n(PS \\{\\
    a\
.\\}
}
/^\\fRLIMITPRIVS\\fR,/ {
    i\
.if \\n(PS \\{\\
    a\
.\\}
}

/^\\fRROLE\\fR,/ {
    i\
.if \\n(SL \\{\\
    a\
.\\}
}
/^\\fRTYPE\\fR,/ {
    i\
.if \\n(SL \\{\\
    a\
.\\}
}
