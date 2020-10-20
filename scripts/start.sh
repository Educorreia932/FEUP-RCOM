#!/bin/bash

tmux -2 \
    new-session  "./initialize.sh ; read" \; \
    split-window -t 0 -v "./server.sh ; read" \; \
    split-window -t 1 -h "./client.sh; read" \; \