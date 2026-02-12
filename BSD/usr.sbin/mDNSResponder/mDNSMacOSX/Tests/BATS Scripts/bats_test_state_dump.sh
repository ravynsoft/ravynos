#!/bin/sh
#
# bats_test_state_dump.sh
# mDNSResponder Tests
#
# Copyright (c) 2019 Apple Inc. All rights reserved.


# trigger state dump
function triger_state_dump {
    local command_line="$1"
    output="$($command_line)"
    if [[ $? -ne 0 ]]; then
        printf "dns-sd -O exit with non-zero return value, the returned error message is:\n"
        echo $output
        return 1
    fi
    return 0
}

# trigger state dump and check if the file is created successfully
function triger_state_dump_and_check_file {
    # $1 is the command line used to trigger state dump
    triger_state_dump "$1"
    if [[ $? -ne 0 ]]; then
        return 1
    fi
    # the returned result is like the following:
    # State Dump Is Saved to: /private/var/log/mDNSResponder/mDNSResponder_state_dump_2019-03-01_17-07-10-260-08-00.txt
    #              Time Used: 33 ms
    # splited the result with '\n'
    IFS=$'\n' read -r -d '' -a line_being_read <<< "$output"
    # ${line_being_read[0]} contains "State Dump Is Saved to: /private/var/log/mDNSResponder/mDNSResponder_state_dump_2019-03-01_17-07-10-260-08-00.txt"
    IFS=":" read -r -d '' -a line_being_splitted <<< "${line_being_read[0]}"
    # get the path "/private/var/log/mDNSResponder/mDNSResponder_state_dump_2019-03-01_17-07-10-260-08-00.txt"
    file_name=$(echo "${line_being_splitted[1]}" | xargs)
    # check if the file exists in the disk
    if [ ! -f $file_name ]; then
        printf "State dump is not created under %s\n" $file_name
        return 1
    fi
    return 0
}

# verify that the state dump contains the full content
function verify_state_dump_content {
    # the passed parameter is the file content to be checked
    local file="$1"
    # the first line of file should start with "---- BEGIN STATE LOG ----"
    local file_start_string="---- BEGIN STATE LOG ----"
    if [[ ! "$file" == "$file_start_string"* ]]; then
        printf "State dump file does not start with %s\n" "$file_start_string"
        return 1
    fi

    # the last line of file should start with "----  END STATE LOG  ----"
    local last_line=$(echo "$file" | tail -n1)
    local file_end_string="----  END STATE LOG  ----"
    if [[ ! "$last_line" == "$file_end_string"* ]]; then
        printf "State dump file does not end with %s\n" "$file_end_string"
        return 1
    fi

    return 0
}

function test_check_dump_directory {
    local dump_path="/private/var/log/mDNSResponder"
    if [ ! -d "$dump_path" ]; then
        printf "Directory \"%s\" does not exist\n" $dump_path
        return 1
    fi
    local permission=$(stat -f "%OLp" $dump_path)
    if [ ! $permission == "755" ]; then
        printf "Directory \"%s\" has incorrect permission. expected=755; actual=%s\n" $dump_path $permission
        return 1
    fi
    local owner=$(ls -ld $dump_path | awk '{print $3}')
    if [ ! $owner == "_mdnsresponder" ]; then
        printf "Directory \"%s\" has incorrect owner. expected=_mdnsresponder; actual=%s\n" $dump_path "$owner"
        return 1
    fi
    return 0
}

function test_output_to_plain_txt_file {
    triger_state_dump_and_check_file "dns-sd -O"
    if [[ $? -ne 0 ]]; then
        return 1
    fi
    # get the file content
    local file_content=$(cat $file_name)
    verify_state_dump_content "$file_content" && rm "$file_name"
    return $?
}

# tests if "dns-sd -O -compress" works as expected
function test_output_to_archive {
    triger_state_dump_and_check_file "dns-sd -O -compress"
    if [[ $? -ne 0 ]]; then
        return 1
    fi
    # get the uncompressed file name
    local file_name_uncompressed=$(tar -tf "$file_name")
    # unzip the file
    tar -xf "$file_name" --directory /tmp/
    # get the file content
    local file_content=$(cat "/tmp/$file_name_uncompressed")
    verify_state_dump_content "$file_content" && rm "$file_name" && rm "/tmp/$file_name_uncompressed"
    return $?
}

# tests if "dns-sd -O -stdout" works as expected
function test_output_to_stdout {
    triger_state_dump "dns-sd -O -stdout"
    if [[ $? -ne 0 ]]; then
        return 1
    fi
    # delete the last line of output, which is "             Time Used: <time> ms"
    output=$(echo "$output" | sed '$d')
    verify_state_dump_content "$output"
    return $?
}

# tests whether the state dump will create at most MAX_NUM_DUMP_FILES, to avoid wasting too much space.
function test_dump_limit {
    # calls "dns-sd -O -compress" for 10 times
    local counter=1
    while [ $counter -le 10 ]
    do
        triger_state_dump_and_check_file "dns-sd -O -compress"
        if [[ $? -ne 0 ]];then
            return 1;
        fi
        ((counter++))
    done

    # $file_name is already initialized when we call "dns-sd -O -compress" above
    local directory=$(dirname "$file_name")
    # get the number of files under $directory
    local file_count=$(ls -Uba1 "$directory" | grep ^mDNSResponder_state_dump_ | wc -l | xargs)
    # clean up the directory
    rm -rf ${directory}/*
    # the number of files should be MAX_NUM_DUMP_FILES, which is defined as 5 in mDNSResponder
    if [[ $file_count -eq 5 ]]; then
        return 0
    else
        return 1
    fi
}

ret=0
# Functions are put inside an array, use ($test) to evaluate it
declare -a tests=("test_check_dump_directory"
                  "test_output_to_plain_txt_file"
                  "test_output_to_archive"
                  "test_output_to_stdout"
                  "test_dump_limit")
echo ""
echo "----State Dump Test Start, $(date)----"
for test in "${tests[@]}"; do
    echo "running $test:"
    ($test)
    if [[ $? -eq 0 ]]; then
        echo "passed"$'\n' # use $'\n' to print one more newline character
    else
        ret=1
        echo "failed"$'\n'
    fi
done
echo "----State Dump Test End, $(date)----"
exit $ret
