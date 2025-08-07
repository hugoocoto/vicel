# Visual Cell Editor

## About

Create, read and write cells.

![image](./image.png)

## Load files

From the terminal, the first argument that is not a flag would be used as 
the filename: `vicel data.csv`

## Flags - Options
- `-m`, `--use-mouse`: Enable mouse support

## Mappings

> At the time of writting this

- `q`, a_quit
- `r`, render
- `gl`, a_add_col
- `gj`, a_add_row
- `j`, a_move_cursor_down
- `k`, a_move_cursor_up
- `h`, a_move_cursor_left
- `l`, a_move_cursor_right
- `v`, a_select_toggle_cell
- `i`, get_set_cell_input
- `I`, get_set_selection_input
- `d`, a_set_cell_type_empty (also known as delete)
- `J`, expand current cell down and move cursor
- `K`, expand current cell up and move cursor
- `H`, expand current cell left and move cursor
- `L`, expand current cell right and move cursor
- `gij`, insert text and move cursor down
- `gik`, insert text and move cursor up
- `gih`, insert text and move cursor left
- `gil`, insert text and move cursor right
- `$`, go to the last cell of the cursor's row
- `^`, go to the first cell of the cursor's row
- `gg`, go to the first cell of the cursor's column
- `G`, go to the last cell of the cursor's column
- `g0`, same as `gg` and `^`

## Input text
Place the cursor above a cell and press `i`.

### Numbers
A number with an optional decimal part separated by a dot.

### Text
Text

### Formula
Text that starts with a `=`. It can contain numbers, or cell identifiers. Cell
identifiers are one or more alpha followed by one or more numbers. FE: `= C2 +
8.5`

> [!CAUTION]
> It's kinda usable for now. 

## Latest Version
[Latest named version](./version.txt)

## Flexing on lines
[Lines written](./wc.md). Little refflexion: the few lines the better, I try
to flex on how little amount of lines my fully functional program has. (It is
the idea).
