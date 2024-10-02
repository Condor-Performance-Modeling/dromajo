#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <env_variable_name>"
    exit 1
fi

ENV_VAR_NAME=$1

ENV_VAR_VALUE=$(eval echo \$$ENV_VAR_NAME)

if [ -z "$ENV_VAR_VALUE" ]; then
    echo "Error: $ENV_VAR_NAME environment variable is not set."
    echo "Please set it to the RISC-V toolchain installation directory."
    echo "IMPORTANT! After setting the variable," \
    "remember to reconfigure the build using cmake .."
    exit 1
fi

echo "$ENV_VAR_NAME environment variable is set to: $ENV_VAR_VALUE"
echo "IMPORTANT! After setting the variable," \
"remember to reconfigure the build using cmake .."
exit 0
