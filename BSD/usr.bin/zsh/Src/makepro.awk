#
# makepro.awk - generate prototype lists
#

BEGIN {
    aborting = 0

    # arg 1 is the name of the file to process
    # arg 2 is the name of the subdirectory it is in
    if(ARGC != 3) {
	aborting = 1
	exit 1
    }
    name = ARGV[1]
    gsub(/^.*\//, "", name)
    gsub(/\.c$/, "", name)
    name = ARGV[2] "_" name
    gsub(/\//, "_", name)
    ARGC--

    printf "E#ifndef have_%s_globals\n", name
    printf "E#define have_%s_globals\n", name
    printf "E\n"
}

# all relevant declarations are preceded by "/**/" on a line by itself

/^\/\*\*\/$/ {
    # The declaration is on following lines.  The interesting part might
    # be terminated by a `{' (`int foo(void) { }' or `int bar[] = {')
    # or `;' (`int x;').
    line = ""
    isfunc = 0
    while(1) {
	if(getline <= 0) {
	    aborting = 1
	    exit 1
	}
	if (line == "" && $0 ~ /^[ \t]*#/) {
            # Directly after the /**/ was a preprocessor line.
            # Spit it out and re-start the outer loop.
	    printf "E%s\n", $0
	    printf "L%s\n", $0
	    next
	}
	gsub(/\t/, " ")
	line = line " " $0
	gsub(/\/\*([^*]|\*+[^*\/])*\*+\//, " ", line)
	if(line ~ /\/\*/)
	    continue
	# If it is a function definition, note so.
	if(line ~ /\) *(VA_DCL )*[{].*$/) #}
	    isfunc = 1
	if(sub(/ *[{;].*$/, "", line)) #}
	    break
    }
    if (!match(line, /VA_ALIST/)) {
	# Put spaces around each identifier.
	while(match(line, /[^_0-9A-Za-z ][_0-9A-Za-z]/) ||
	      match(line, /[_0-9A-Za-z][^_0-9A-Za-z ]/))
	    line = substr(line, 1, RSTART) " " substr(line, RSTART+1)
    }
    # Separate declarations into a type and a list of declarators.
    # In each declarator, "@{" and "@}" are used in place of parens to
    # mark function parameter lists, and "@!" is used in place of commas
    # in parameter lists.  "@<" and "@>" are used in place of
    # non-parameter list parens.
    gsub(/ _ +/, " _ ", line)
    while(1) {
	if(isfunc && match(line, /\([^()]*\)$/))
	    line = substr(line, 1, RSTART-1) " _ (" substr(line, RSTART) ")"
	else if(match(line, / _ \(\([^,()]*,/))
	    line = substr(line, 1, RSTART+RLENGTH-2) "@!" substr(line, RSTART+RLENGTH)
	else if(match(line, / _ \(\([^,()]*\)\)/))
	    line = substr(line, 1, RSTART-1) "@{" substr(line, RSTART+5, RLENGTH-7) "@}" substr(line, RSTART+RLENGTH)
	else if(match(line, /\([^,()]*\)/))
	    line = substr(line, 1, RSTART-1) "@<" substr(line, RSTART+1, RLENGTH-2) "@>" substr(line, RSTART+RLENGTH)
	else
	    break
    }
    sub(/^ */, "", line)
    match(line, /^((const|enum|mod_export|static|struct|union|volatile) +)*([_0-9A-Za-z]+ +|((char|double|float|int|long|short|unsigned|void) +)+)((const|static) +)*/)
    dtype = substr(line, 1, RLENGTH)
    sub(/ *$/, "", dtype)
    if(" " dtype " " ~ / static /)
	locality = "L"
    else
	locality = "E"
    exported = " " dtype " " ~ / mod_export /
    line = substr(line, RLENGTH+1) ","
    # Handle each declarator.
    if (match(line, /VA_ALIST/)) {
	# Already has VARARGS handling.

	# Put parens etc. back
	gsub(/@[{]/, "((", line)
	gsub(/@}/, "))", line)
	gsub(/@</, "(", line)
	gsub(/@>/, ")", line)
	gsub(/@!/, ",", line)
	sub(/,$/, ";", line)
	gsub(/mod_export/, "mod_import_function", dtype)
	gsub(/VA_ALIST/, "VA_ALIST_PROTO", line)
	sub(/ VA_DCL/, "", line)

	if(locality ~ /E/)
	    dtype = "extern " dtype

	if (match(line, /[_0-9A-Za-z]+\(VA_ALIST/))
	  dnam = substr(line, RSTART, RLENGTH-9)

	# If this is exported, add it to the exported symbol list.
	if (exported)
	    printf "X%s\n", dnam

	printf "%s%s %s\n", locality, dtype, line
    } else {
	while(match(line, /^[^,]*,/)) {
		# Separate out the name from the declarator.  Use "@+" and "@-"
		# to bracket the name within the declarator.  Strip off any
		# initialiser.
		dcltor = substr(line, 1, RLENGTH-1)
		line = substr(line, RLENGTH+1)
		sub(/=.*$/, "", dcltor)
		match(dcltor, /^([^_0-9A-Za-z]| const )*/)
		dcltor = substr(dcltor, 1, RLENGTH) "@+" substr(dcltor, RLENGTH+1)
		match(dcltor, /^.*@\+[_0-9A-Za-z]+/)
		dcltor = substr(dcltor, 1, RLENGTH) "@-" substr(dcltor, RLENGTH+1)
		dnam = dcltor
		sub(/^.*@\+/, "", dnam)
		sub(/@-.*$/, "", dnam)

		# Put parens etc. back
		gsub(/@[{]/, " _((", dcltor)
		gsub(/@}/, "))", dcltor)
		gsub(/@</, "(", dcltor)
		gsub(/@>/, ")", dcltor)
		gsub(/@!/, ",", dcltor)

		# If this is exported, add it to the exported symbol list.
		if(exported)
		    printf "X%s\n", dnam

		# Format the declaration for output
		dcl = dtype " " dcltor ";"
		if(locality ~ /E/)
		    dcl = "extern " dcl
		if(isfunc)
		    gsub(/ mod_export /, " mod_import_function ", dcl)
		else
		    gsub(/ mod_export /, " mod_import_variable ", dcl)
		gsub(/@[+-]/, "", dcl)
		gsub(/ +/, " ", dcl)
		while(match(dcl, /[^_0-9A-Za-z] ./) || match(dcl, /. [^_0-9A-Za-z]/))
		    dcl = substr(dcl, 1, RSTART) substr(dcl, RSTART+2)
		printf "%s%s\n", locality, dcl
	}
    }
}

END {
    if(aborting)
	exit 1
    printf "E\n"
    printf "E#endif /* !have_%s_globals */\n", name
}
