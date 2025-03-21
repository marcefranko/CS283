#!/usr/bin/env bats

@test "Local command check" {
    run "./dsh" <<EOF
ls -la
exit
EOF

    echo "Captured stdout:"
    echo "$output"
    echo "Exit Status: $status"
    
    [[ "$output" == *"local mode"* ]]
    [[ "$output" == *"dshlib.c"* ]]
    [[ "$output" == *"rshlib.h"* ]]
    
    [ "$status" -eq 0 ]
}

@test "Local Pipe Command" {
    run "./dsh" <<EOF
ls | grep .c
exit
EOF

    echo "Captured stdout:"
    echo "$output"
    echo "Exit Status: $status"
    
    [[ "$output" == *"local mode"* ]]
    [[ "$output" == *"dshlib.c"* ]]
    [[ "$output" == *"dsh_cli.c"* ]]
    
    [ "$status" -eq 0 ]
}

@test "Local Built-in Command (cd)" {
    run "./dsh" <<EOF
cd ..
pwd
exit
EOF

    echo "Captured stdout:"
    echo "$output"
    echo "Exit Status: $status"
    
    [[ "$output" == *"local mode"* ]]
    [[ "$output" != *"$(basename $(pwd))"* ]]
    
    [ "$status" -eq 0 ]
}


@test "Single Threaded  Basic Command" {
    ./dsh -s -p 5679 &
    server_pid=$!
    
    sleep 1
    
    run "./dsh" -c -p 5679 <<EOF
ls -la
exit
EOF

    ./dsh -c -p 5679 <<EOF
stop-server
EOF
    
    wait $server_pid
    
    echo "Client output:"
    echo "$output"
    echo "Client exit status: $status"
    
    [[ "$output" == *"dshlib.c"* ]]
    [[ "$output" == *"rshlib.h"* ]]
    
    [ "$status" -eq 0 ]
}

@test "MultiThreaded Server Start/Stoppage" {
    ./dsh -s -x -p 5680 &
    server_pid=$!
    
    sleep 1
    
    run "./dsh" -c -p 5680 <<EOF
stop-server
EOF

    wait $server_pid
    server_status=$?
    
    echo "Server exit status: $server_status"
    echo "Client output:"
    echo "$output"
    echo "Client exit status: $status"
    
    [ "$server_status" -eq 0 ]
    [ "$status" -eq 0 ]
    [[ "$output" == *"stop"* ]]
}

@test "MultiThreaded Numerou Client Check" {
    ./dsh -s -x -p 5681 &
    server_pid=$!
    
    sleep 1
    
    temp_dir=$(mktemp -d)
    client1_output="$temp_dir/client1.out"
    client2_output="$temp_dir/client2.out"
    
    (echo "echo client1_test" | ./dsh -c -p 5681 > "$client1_output" 2>&1) &
    client1_pid=$!
    
    (echo "echo client2_test" | ./dsh -c -p 5681 > "$client2_output" 2>&1) &
    client2_pid=$!
    
    wait $client1_pid
    wait $client2_pid
    
    ./dsh -c -p 5681 <<EOF
stop-server
EOF

    wait $server_pid
    server_status=$?
    
    client1_content=$(cat "$client1_output")
    client2_content=$(cat "$client2_output")
    
    echo "Server exit status: $server_status"
    echo "Client 1 output: $client1_content"
    echo "Client 2 output: $client2_content"
    
    rm -r "$temp_dir"
    
    [[ "$client1_content" == *"client1_test"* ]]
    [[ "$client2_content" == *"client2_test"* ]]
    
    [ "$server_status" -eq 0 ]
}
