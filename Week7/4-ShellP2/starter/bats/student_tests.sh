#!/usr/bin/env bats

# File: student_tests.sh
# 
# Create your unit tests suit in this file
@test "Example: check ls runs without errors" {
    current=$(pwd)

    cd /tmp
    mkdir -p dsh-test

    run "${current}/dsh" <<EOF                
ls
EOF

    # Assertions
    [ "$status" -eq 0 ]
}

@test "No command provided" {
    current=$(pwd)

    cd /tmp
    mkdir -p dsh-test

    run "${current}/dsh" <<EOF                

EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="dsh2>warning:nocommandsprovideddsh2>cmdloopreturned0"

    # These echo commands will help with debugging and will only print
    #if the test fails
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]

}

@test "cd to an unexistent directory" {
    current=$(pwd)

    cd /tmp
    mkdir -p dsh-test

    run "${current}/dsh" <<EOF                
cd dir_not_existant
pwd
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="cd:'dir_not_existant'doesnotexists/tmpdsh2>dsh2>dsh2>cmdloopreturned0"
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "provide more than one argument to cd" {
    current=$(pwd)

    cd /tmp
    mkdir -p dsh-test

    run "${current}/dsh" <<EOF                
cd dsh-test arg1
pwd
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '[:space:]')

    # Expected output with all whitespace removed for easier matching
    expected_output="cd:toomanyarguments/tmpdsh2>dsh2>dsh2>cmdloopreturned0"
    echo "Captured stdout:" 
    echo "Output: $output"
    echo "Exit Status: $status"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Handles leading, trailing, and multiple spaces correctly" {
    current=$(pwd)

    cd /tmp
    mkdir -p dsh-test

    run "${current}/dsh" <<EOF
      echo    hello   "   booo ooys"
EOF

    # Strip all whitespace (spaces, tabs, newlines) from the output
    stripped_output=$(echo "$output" | tr -d '\t\n\r\f\v')

    # Expected output with all whitespace removed for easier matching
    expected_output="hello    booo ooysdsh2> dsh2> cmd loop returned 0"
    echo "Captured stdout:" 
    echo "Output: $stripped_output"
    echo "Exit Status: $status"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "exit command exits dsh2" {
    current=$(pwd)
    
    cd /tmp
    mkdir -p dsh-test
    
    run "${current}/dsh" <<EOF 
exit
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="dsh2>"

    echo "Captured stdout:" 
    echo "Output: $stripped_output"
    echo "Exit Status: $status"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "rc command prints the return number of the previous command" {
    current=$(pwd)
    
    cd /tmp
    mkdir -p dsh-test
    
    run "${current}/dsh" <<EOF  
cd one_dir
rc
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="cd:'one_dir'doesnotexistsdsh2>dsh2>-6dsh2>cmdloopreturned0"

    echo "Captured stdout:" 
    echo "Output: $stripped_output"
    echo "Exit Status: $status"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}

@test "Invalid command handling" {
    current=$(pwd)
    
    cd /tmp
    mkdir -p dsh-test
    
    run "${current}/dsh" <<EOF                
invalidcommand
EOF

    stripped_output=$(echo "$output" | tr -d '[:space:]')
    expected_output="invalidcommand:commandnotfounddsh2>dsh2>dsh2>cmdloopreturned0"

    echo "Captured stdout:" 
    echo "Output: $stripped_output"
    echo "Exit Status: $status"

    # Check exact match
    [ "$stripped_output" = "$expected_output" ]

    # Assertions
    [ "$status" -eq 0 ]
}