#!/usr/bin/env bash
# This script processes symbols output by Gallium using glibc to human-readable function names

lastbin=
i=-1
dir="$(mktemp -d)"
input="$1"

# Gather all unique addresses for each binary
sed -nre 's|([^ ]*/[^ ]*)\(\+0x([^)]*).*|\1 \2|p' "$input"|sort|uniq|while read bin addr; do
	if test "$lastbin" != "$bin"; then
		((++i))
		lastbin="$bin"
		echo "$bin" > "$dir/$i.addrs.bin"
	fi
	echo "$addr" >> "$dir/$i.addrs"
done

# Construct a sed script to convert hex address to human readable form, and apply it
for i in "$dir"/*.addrs; do
	bin="$(<"$i.bin")"
	addr2line -p -e "$bin" -a -f < "$i"|sed -nre 's@^0x0*([^:]*): ([^?]*)$@s|'"$bin"'(+0x\1)|\2|g@gp'
	rm -f "$i" "$i.bin"
done|sed -f - "$input"

rmdir "$dir"
