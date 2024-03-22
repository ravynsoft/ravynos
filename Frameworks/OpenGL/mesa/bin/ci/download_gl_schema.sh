#!/bin/sh

# Helper script to download the schema GraphQL from Gitlab to enable IDEs to
# assist the developer to edit gql files

SOURCE_DIR=$(dirname "$(realpath "$0")")

(
    cd $SOURCE_DIR || exit 1
    gql-cli https://gitlab.freedesktop.org/api/graphql --print-schema > schema.graphql
)
