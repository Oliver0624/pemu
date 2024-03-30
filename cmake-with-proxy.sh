#!/bin/bash
if [ "$1" == "--version" ]; then
    cmake "$1"
else
    source /home/luchang/start_proxy.sh && cmake "$@"
fi