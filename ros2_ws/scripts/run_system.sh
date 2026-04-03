#!/bin/bash

MODE=$1
NODE_NAME=$2
LAUNCH_FILE=$3

if [ "$NODE_NAME" == "" ] || [ "$LAUNCH_FILE" == "" ] || [ "$MODE" == "" ]; then
    echo "Error: Missing arguments!"
    echo "Syntax: ./run_system.sh <wifi|eth> <node_name> <name_of_launch_file>"
    exit 1
fi

if [ "$MODE" == "eth" ]; then
    echo "Choose mode Ethernet..."
    export ROS_DISCOVERY_SERVER="10.42.0.2:11811"

elif [ "$MODE" == "wifi" ]; then
    echo "Choose mode WIFI..."
    export ROS_DISCOVERY_SERVER="192.168.2.203:11811"

else
    echo "Error: Please choose network! Syntax: ./run_system.sh wifi <name_of_launch> or ./run_system.sh eth <name_of_launch>"
    exit 1
fi

# Clean old Deamon
ros2 daemon stop

# Run file system
echo "Starting Node..."
ros2 launch $NODE_NAME $LAUNCH_FILE
