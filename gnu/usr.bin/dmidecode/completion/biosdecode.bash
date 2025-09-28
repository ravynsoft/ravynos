# bash completion for biosdecode                           -*- shell-script -*-

_comp_cmd_biosdecode() {
	local cur prev
	COMPREPLY=()
	cur=${COMP_WORDS[COMP_CWORD]}
	prev=${COMP_WORDS[COMP_CWORD - 1]}

	case $prev in
	-d | --dev-mem)
		: "${cur:=/dev/}"
		local IFS=$'\n'
		compopt -o filenames
		COMPREPLY=($(compgen -f -- "$cur"))
		return 0
		;;
	--pir)
		COMPREPLY=($(compgen -W '
			full
		' -- "$cur"))
		return 0
		;;
	-[hV] | --help | --version)
		return 0
		;;
	esac

	if [[ $cur == -* ]]; then
		COMPREPLY=($(compgen -W '
			--dev-mem
			--pir
			--help
			--version
		' -- "$cur"))
		return 0
	fi

} && complete -F _comp_cmd_biosdecode biosdecode

# ex: filetype=sh
