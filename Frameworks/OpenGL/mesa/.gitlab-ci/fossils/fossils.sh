#!/usr/bin/env bash
# shellcheck disable=SC2086 # we want word splitting
# shellcheck disable=SC2155

FOSSILS_SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
FOSSILS_YAML="$(readlink -f "$1")"
FOSSILS_RESULTS="$2"

clone_fossils_db()
{
    local repo="$1"
    local commit="$2"
    rm -rf fossils-db
    git clone --no-checkout "$repo" fossils-db
    (cd fossils-db || return; git reset "$commit" || git reset "origin/$commit")
}

query_fossils_yaml()
{
    python3 "$FOSSILS_SCRIPT_DIR/query_fossils_yaml.py" \
        --file "$FOSSILS_YAML" "$@"
}

create_clean_git()
{
    rm -rf .clean_git
    cp -R .git .clean_git
}

restore_clean_git()
{
    rm -rf .git
    cp -R .clean_git .git
}

fetch_fossil()
{
    local fossil="${1//,/?}"
    echo -n "[fetch_fossil] Fetching $1... "
    local output=$(git lfs pull -I "$fossil" 2>&1)
    local ret=0
    if [[ $? -ne 0 || ! -f "$1" ]]; then
        echo "ERROR"
        echo "$output"
        ret=1
    else
        echo "OK"
    fi
    restore_clean_git
    return $ret
}

if [[ -n "$(query_fossils_yaml fossils_db_repo)" ]]; then
    clone_fossils_db "$(query_fossils_yaml fossils_db_repo)" \
                     "$(query_fossils_yaml fossils_db_commit)"
    cd fossils-db || return
else
    echo "Warning: No fossils-db entry in $FOSSILS_YAML, assuming fossils-db is current directory"
fi

# During git operations various git objects get created which
# may take up significant space. Store a clean .git instance,
# which we restore after various git operations to keep our
# storage consumption low.
create_clean_git

for fossil in $(query_fossils_yaml fossils)
do
    fetch_fossil "$fossil" || exit $?
    if ! fossilize-replay --num-threads 4 $fossil 1>&2 2> $FOSSILS_RESULTS/fossil_replay.txt;
    then
        echo "Replay of $fossil failed"
        grep "pipeline crashed or hung" $FOSSILS_RESULTS/fossil_replay.txt
        exit 1
    fi
    rm $fossil
done

exit $ret
