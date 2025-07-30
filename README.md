# Visual Cell Editor

## About

Create, read and write files based on cells. (<- this is the idea)

![image](./image.png)

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

## Input text

Place the cursor above a cell and press `i`.

### Numbers
A number with an optional decimal part separated by a dot.

### Text
Text

### Formula
Text that starts with a `=`. It can contain numbers, or cell identifiers. Cell 
identifiers are one or more alpha followed by one or more numbers. FE: `= C2 + 8.5`

> [!CAUTION]
> It's not usable for now. (Well, if you need more than a sum)
