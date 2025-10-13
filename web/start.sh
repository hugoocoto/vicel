#!/usr/bin/env bash

cd ..

make &>/dev/null
echo "http://$(hostname -i):8080"
ttyd -W -p 8080 tmux new-session vicel 2>/dev/null
