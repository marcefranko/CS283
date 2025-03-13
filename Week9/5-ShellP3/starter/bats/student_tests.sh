#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file

@test "Example: check ls runs without errors" {
    run ./dsh <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Emtpy command warns user" {
    run "./dsh" <<EOF 
      
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh3>warning:nocommandsprovideddsh3>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Attempting to input a command whose executable is more than 64 chars causes an error" {
    run "./dsh" <<EOF 
thisisaverylongcommandnamethatexceedssixtyfourcharactersanddoesnotexist
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="Commanderror:executableexceeds64charactersdsh3>dsh3>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Attempting to input a command whose args exceed 256 chars causes an error" {
    run "./dsh" <<EOF 
thiscommanddoesnotexist "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="Commanderror:argumentsexceed256charactersdsh3>dsh3>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Commands can have a maximum of 9 arguments including executable" {
    run "./dsh" <<EOF 
the_executable arg1 arg2 arg3 arg4 arg5 arg6 arg7 arg8 arg9
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="Commanderror:commandhastoomanyargumentsdsh3>dsh3>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Executing non-piped commands work as expected" {
    run "./dsh" <<EOF 
ls
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="batsdragon.cdshdsh_cli.cdshlib.cdshlib.hmakefiledsh3>dsh3>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Piping is limited to 8 commands" {
    run "./dsh" <<EOF 
these | are | nine | random | commands | for | this | testcase | Bruh it was almost perfect
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh3>error:pipinglimitedto8commandsdsh3>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Piping works with built-in commands" {
    run "./dsh" <<EOF 
dragon | wc -l
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="38dsh3>dsh3>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "rc command returns correct exit code of previous commands" {
    run "./dsh" <<EOF 
cd bats
rc
cd hello
rc
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="cd:'hello'doesnotexistsdsh3>dsh3>0dsh3>dsh3>-6dsh3>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "exiting the shell works as expected" {
    run "./dsh" <<EOF 
exit
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh3>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"
    echo "${stripped_output} -> ${expected_output}"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}