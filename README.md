# Visual Cell Editor
Webpage: https://hugoocoto.github.io/vicel/
![Screenshot](images/image.png)

## Features
* Edit CSV-like grids interactively in your terminal
* Supports numbers, text, and formulas
* Keyboard-focused Vim-style motions
* Lightweight and minimal code
* Configuration via .toml file
* Autosave
* Mouse support

## Reference manual 
Documentation and installation/usage guide are
[here](./docs/vicel_reference.pdf). 

## Configuration
You can modify some values using the vicel config file. This file is searched in
the following paths: `./vicel.toml`, `~/vicel.toml`, `~/.config/vicel.toml`,
`~/.config/vicel/vicel.toml`. If other file is desired to be used it can be
specified with the `-c` or `--config-file` flag, following with the full path of
the config file. The format chosen for the file is [toml](https://toml.io). 

The valid fields whith their default values are set [here](#vicel.toml).

## Latest Version
[See version](./version.txt)

## Lines of Code
[Lines written](./wc.md) A little reflection: the fewer the lines, the better.
I like to flex on how little code is needed for a fully functional program.

## Status
> It’s **kinda usable** for now.

![](https://wakatime.com/badge/user/2a7b4567-ab1f-4fb2-98ff-2b3fdbf94654/project/98a99176-d7a4-48ba-968c-4e410787a98f.svg)

## Known issues
Issues [here](./TODO_ISSUES.md).

## Sheets done by the community
* *Horario* by @hugoocoto ![](images/image3.png)

