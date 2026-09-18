#!/usr/bin/env bash

make &>/dev/null
cd ..
make &>/dev/null
echo "http://$(hostname -i):8080"
ttyd -W -p 8080 ./web/launcher vicel -c web/theme-web.lua
rm -f ./web/launcher
