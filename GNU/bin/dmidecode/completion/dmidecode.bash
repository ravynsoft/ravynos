# bash completion for dmidecode                            -*- shell-script -*-

_comp_cmd_dmidecode() {
	local cur prev
	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD - 1]}

	case $prev in
	-d | --dev-mem | --dump-bin | --from-dump)
		if [[ $prev == -d || $prev == --dev-mem ]]; then
			: "${cur:=/dev/}"
		fi
		local IFS=$'\n'
		compopt -o filenames
		COMPREPLY=($(compgen -f -- "$cur"))
		return 0
		;;
	-s | --string)
		COMPREPLY=($(compgen -W '$("$1" --list-strings)' -- "$cur"))
		return 0
		;;
	-t | --type)
		COMPREPLY=($(compgen -W '$("$1" --list-types)' -- "$cur"))
		return 0
		;;
	--dump-bin | --from-dump)
		local IFS=$'\n'
		compopt -o filenames
		COMPREPLY=($(compgen -f -- "$cur"))
		return 0
		;;
	-[hVH] | --help | --version | --handle | --oem-string)
		return 0
		;;
	esac

	if [[ $cur == -* ]]; then
		COMPREPLY=($(compgen -W '
			--dev-mem
			--help
			--quiet
			--string
			--list-strings
			--type
			--list-types
			--handle
			--dump
			--dump-bin
			--from-dump
			--no-sysfs
			--oem-string
			--version
		' -- "$cur"))
		return 0
	fi

} && complete -F _comp_cmd_dmidecode dmidecode

# ex: filetype=sh
