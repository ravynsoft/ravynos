#!/bin/zsh -f

local file

if ((! ARGC)) then
	set -- -
fi

for file
do
	if [[ "$file" == - ]] then
		while read -u0ek 4096; do ; done
	else
		while read -u0ek 4096; do ; done < "$file"
	fi
done
