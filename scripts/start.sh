#!/bin/bash

tmux -2 \
    new-session  "./initialize.sh ; bash" \; \
    split-window -t 0 -v "./server.sh ; bash" \; \
    split-window -t 1 -h "./client.sh; bash" \; \