.text
foo:
	troo	%r6,%r9,15
	troo	%r6,%r9
	trot	%r6,%r9,15
	trot	%r6,%r9
	trto	%r6,%r9,15
	trto	%r6,%r9
	trtt	%r6,%r9,15
	trtt	%r6,%r9
# z9-109 z/Architecture mode extended sske with an additional parameter
# make sure the old version still works for esa
	sske	%r6,%r9
