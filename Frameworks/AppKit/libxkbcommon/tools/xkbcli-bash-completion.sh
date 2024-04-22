# bash completion support for xkbcli.

# See completion API documentation: https://github.com/scop/bash-completion
# NOTE: The script parses the commands help messages to provide the completions,
#       thus any new subcommand or option will be supported, as long as it has its
#       entry in the help messages. This should result in low maintenancei effort.

___xkbcli_main()
{
    # Initialization: https://github.com/scop/bash-completion/blob/fdf4456186eb4548ef628e65fb1be73d8e4695e9/bash_completion.d/000_bash_completion_compat.bash#L205
    local cur prev words cword cmd
    _init_completion -s || return

    # Find subcommand
    local i=1
    while [[ "$i" -lt "$COMP_CWORD" ]]; do
        local s="${COMP_WORDS[i]}"
        case "$s" in
            -*) ;;
            *)
            cmd="$s"
            break
            ;;
        esac
        (( i++ ))
    done

    # Parse available subcommands
    local line
    local is_command_list=false
    local subcommands=()
    while IFS='' read -r line; do
        # Traverse subcommand list
        if [[ "$is_command_list" == true ]]; then
            # Check for subcommand based on the indentation
            if [[ "$line" =~ ^[[:blank:]]{2}([[:alpha:]]([[:alnum:]]|-)+)$ ]]; then
                subcommands+=("${BASH_REMATCH[1]}")
            # Detect end of subcommand list based on indentation
            elif [[ "$line" =~ ^[[:graph:]] ]]; then
                is_command_list=false
            fi
        # Detect start of subcommand list
        elif [[ "$line" == "Commands:" ]]; then
            is_command_list=true
        fi
    # NOTE: <( COMMAND ) Bash construct is “process substitution”.
    done < <(xkbcli --help)

    # No previous subcommand or incomplete: completion for root xkbcli command
    if [[ "$i" -eq "$COMP_CWORD" ]]; then
        local opts
        # Doc for _parse_help: https://github.com/scop/bash-completion/blob/fdf4456186eb4548ef628e65fb1be73d8e4695e9/bash_completion.d/000_bash_completion_compat.bash#L311
        opts=$(_parse_help xkbcli)
        local cur="${COMP_WORDS[COMP_CWORD]}"
        COMPREPLY=($(compgen -W "${subcommands[*]} $opts" -- "$cur"))
        return
    fi

    # Found a supported subcommand: proceed to completion
    if [[ "${subcommands[*]}" =~ (^| )$cmd( |$) ]]; then
        ___xkbcli_subcommand "$cmd"
    fi
}

___xkbcli_subcommand()
{
    # Some special cases
    case $1 in
        compile-keymap | interactive-evdev)
            case ${COMP_WORDS[COMP_CWORD-1]} in
                --include | --keymap)
                    _filedir
                    return;;
            esac
            ;;
        list)
            if [[ ${COMP_WORDS[COMP_CWORD]} != -* ]]; then
                _filedir
                return
            fi
            ;;
    esac

    # Parse help to get command options
    local opts
    # Doc for _parse_usage and _parse_help:
    # • https://github.com/scop/bash-completion/blob/fdf4456186eb4548ef628e65fb1be73d8e4695e9/bash_completion.d/000_bash_completion_compat.bash#L335
    # • https://github.com/scop/bash-completion/blob/fdf4456186eb4548ef628e65fb1be73d8e4695e9/bash_completion.d/000_bash_completion_compat.bash#L311
    # We need both as the current help messages adopt both GNU and BSD styles.
    opts=$(_parse_usage xkbcli "$1 --help")
    opts+=$(_parse_help xkbcli "$1 --help")
    local cur="${COMP_WORDS[COMP_CWORD]}"
    COMPREPLY=($(compgen -W "$opts" -- "$cur"))
}

complete -F ___xkbcli_main xkbcli
