shcat()
{
	while read -r ; do
		printf "%s\n" "$REPLY"
	done
}

if [ -n "$1" ]; then
	shcat < "$1"
else
	shcat
fi
