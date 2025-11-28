# Visual Cell Editor
Vicel is a free open source TUI spreadsheet editor. You can read, modify and
save data stored in rows and columns. It aims to be an alternative to
proprietary non gratis well known Microsoft program, for non professional usage.

Webpage: https://hugocoto.com/vicel.html

![Screenshot](images/image.png)
*The previous image does not show the latest version of vicel.*

## Features
* Edit CSV-like grids interactively in your terminal
* Supports numbers, text, and formulas
* Keyboard-focused Vim-style motions
* Lightweight and minimal code
* Configuration via .py file
* Mouse support

## Reference manual 
Documentation and installation/usage guide are
[here](./docs/vicel_reference.pdf). [Download link](https://raw.githubusercontent.com/hugoocoto/vicel/main/docs/vicel_reference.pdf)

## How to install
You can install it with make. `make` creates the debug mode executable. `make
clean install` creates the release executable an moves it to `~/.local/bin`.

## How to host vicel in the browser
You can use `ttyd` to host a pty in the browser. Using the script
`./web/start.sh` you would have vicel running in http://localhost:8080.
```sh 
cd web 
./start
```

You can find it as a demo here: https://hugocoto.com/vicel

## Lines of Code
[Lines written](./wc) A little reflection: the fewer the lines, the better.
I like to flex on how little code is needed for a fully functional program.

## Status
> It’s **kinda usable** for now.

![](https://wakatime.com/badge/user/2a7b4567-ab1f-4fb2-98ff-2b3fdbf94654/project/98a99176-d7a4-48ba-968c-4e410787a98f.svg)

## Star History

<a href="https://www.star-history.com/#hugoocoto/vicel&type=date&legend=bottom-right">
 <picture>
   <source media="(prefers-color-scheme: dark)" srcset="https://api.star-history.com/svg?repos=hugoocoto/vicel&type=date&theme=dark&legend=bottom-right" />
   <source media="(prefers-color-scheme: light)" srcset="https://api.star-history.com/svg?repos=hugoocoto/vicel&type=date&legend=bottom-right" />
   <img alt="Star History Chart" src="https://api.star-history.com/svg?repos=hugoocoto/vicel&type=date&legend=bottom-right" />
 </picture>
</a>

## Sheets done by the community
* *Horario* by @hugoocoto ![](images/image3.png)

