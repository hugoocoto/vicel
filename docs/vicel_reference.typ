#let author = "Hugo Coto Flórez"
#let mail = "hugo.coto@outlook.com"

#show heading.where(
  level: 1,
): it => block(width: 100%)[
  #set align(center)
  #set text(weight: "regular")
  #smallcaps(it.body)
  #linebreak()
  #linebreak()
]

#set page(header: align(right + horizon)[
  #author (#link("mailto:" + mail))
])

#heading(outlined: false, bookmarked: false)[Vicel Reference Manual]

== Contents
#outline(title: none)

== Abstract
Vicel is a free open source TUI spreadsheet editor. You can read, modify and
save data stored in rows and columns. It aims to be an alternative to privative
non gratis well known Microsoft program, for non professional usage (as it is
quite simple, al least for now).

== About development
For now it is being developed by Hugo Coto as a side project. He do that
because for college reasons he has to use the Windows-only mouse-centered
similar and he almost went ill. The plan for the future is reach a stable
version with all and no more than the useful and needed features to have an
usable program for non professional usage. What I mean with no professional is
that it's not planned to support any graphs neither economic formulas or such
specific things. It's true that with builtins someone can adapt vicel to his own
necessities.

== Open Vicel
As vicel is a TUI program you have to start it from the command line. If you
installed it in #smallcaps("~/.local/bin") then you can start it as a
normal terminal tool:
```sh
vicel filename [options]
```
If the filename is omitted it opens an unnamed sheet. Note that this sheet
can't be saved. Also, you can't open more than one file at the same time.

=== Options
The options supported are the following:

#table(
  columns: 2,
  stroke: none,
  table.header("Command", "Description"),
  table.hline(),
  [`-m`, `--use-mouse`  ], [Enable mouse support],
  [`-D`, `--debug`      ], [Enable debug output ],
  [`-c`, `--config-file`], [Set custom file path],
),

For example, if you want to open the file #smallcaps("./sheets/table.csv") with
a config file in #smallcaps("./config/vicel.toml"), the command line should
look like that:
```sh
vicel sheets/table.csv -c config/vicel.toml
```

== Moving around
As a vim enthusiast, movement is keyboard centered, and use the vim default
#smallcaps("hjkl"). Every action can be prefixed with a number, so it would be
executed that amount of times. The following table describes the basic movement.

#table(
  columns: 2,
  stroke: none,
  table.header("Command", "Description"),
  table.hline(),
  [ `h`, `l`, `j`, `k`], [ Move cursor left, right, down, up],
  [ `$`], [ Go to last cell of the current row],
  [ `^`], [ Go to first cell of the current row],
  [ `gg`], [ Go to first cell of the current column],
  [ `G`], [ Go to last cell of the current column],
  [ `g0`], [ Same as `^` and `gg`],
),

== Write or edit
For write text in a cell, move the cursor there an press `i`. A text input box
would be open at the cell. After write, press #smallcaps("Enter") to save it.
The data type would be automatically calculated by the program. Every number,
with or without a fractional part separated by a dot would be interpreted as a
#smallcaps("Number"). If the text written starts with a equal sign it would be
interpreted as a formula. Other formats would be set to #smallcaps("Text").

#table(
  columns: 2,
  stroke: none,
  table.header("Command", "Description"),
  table.hline(),
  [i], [insert/modify text],
  [d], [delete cell content],
  [v], [toggle cell selection],
),

The valid types are described in the following table by it's formal
representation.

#table(
  columns: 2,
  stroke: none,
  table.header("Type", "Formal"),
  table.hline(),
  [#smallcaps("Number")], [[0-9]+("."[0-9]+)?],
  [#smallcaps("Formula")], ["=" #smallcaps("Formula body")],
  [#smallcaps("Text")], [!#smallcaps("Number") && !#smallcaps("Formula")],
),

=== Formula
Formulas are expressions that evaluate to a valid value. They start with a
equal sign. The function body have to contain a valid expression.

#table(
  columns: 3,
  stroke: none,
  table.header("Type", "Description", "Example"),
  table.hline(),
  [Literal], [Number, text, identifier or range], [see below],
  [Number], [As cell type #smallcaps("Number")], [5987, 45.6],
  [Text], [Alphas or text surrounded by `'`], [hello, '5.9'],
  [Identifier], [Cell reference by name as #smallcaps("ColRow")], [A0, b5, ZZ98],
  [Range], [Cell range as #smallcaps("ID:ID")], [A0:A2, A7:C8],
  [Arithmetic operators], [Evaluate arithmetic expressions], [+, -, /, \*, ^],
  [Comparison operators], [Compare two expressions], [>, <, >=, <=, ==, !=, !],
  [functions], [Reserved names that convert some input in some output, with the form #smallcaps("name(args,...)")],
  [sum(A0,A1)],
  table.hline(),
  [Todo: expand formula reference], [], [],
),

=== Advanced write: write and move
There is a builtin feature to automatically move before insert text. It is
useful if you need to input a big amount of data in a given direction. The idea
is to prefix the following commands with a number, to do it for a given number
of times.

#table(
  columns: 2,
  stroke: none,
  table.header("Command", "Description"),
  table.hline(),
  [gih, gij, gik, gil], [insert text and move in the given direction],
),

== Modify sheet structure
There is some commands to add/delete rows and columns. Note that formula
identifier would not change on row/col insertion/deletion.

#table(
  columns: 2,
  stroke: none,
  table.header("Command", "Description"),
  table.hline(),
  [`g#`], [ Add row/column: see below],
  [`gd#`], [ Delete row/column: see below],
  [`gj`], [ Add a new row after the cursor],
  [`gl`], [ Add a new column after the cursor],
  [`gk`], [ Add a new row before the cursor],
  [`gh`], [ Add a new column before the cursor],
  [`gJ`], [ Add a new row at the end ],
  [`gL`], [ Add a new column at the end],
  [`gK`], [ Add a new row at the start ],
  [`gH`], [ Add a new column at the start],
  [`gdj`], [ Delete row and move up],
  [`gdl`], [ Delete column and move right],
  [`gdk`], [ Delete row and move down],
  [`gdh`], [ Delete column and move left],
)

== Expand cells
There is a feature to get the cell value given the previous one and a direction.
Numbers add 1 and formula identifiers recalculate depending on the direction.
They can avoid modification if `$` is used before identifier letter (for freeze
column) or number (for freeze row). The mappings to do this are described in the
following table.

#table(
  columns: 2,
  stroke: none,
  table.header("Command", "Description"),
  table.hline(),
  [`J`, `K`, `H`, `L`], [Expand current cell down, up, left, right (and move)],
)

== Copy - Paste
As a vim user, you would want to copy-paste things around. Unfortunately, it's
only possible to copy a single cell value and paste it in a single cell. 

#table(
  columns: 2,
  stroke: none,
  table.header("Command", "Description"),
  table.hline(),
  [p], [paste],
  [y], [yank (copy)],
)

== Other commands
There are another useful commands, described below.

#table(
  columns: 2,
  stroke: none,
  table.header("Command", "Description"),
  table.hline(),
  [q], [Save and quit],
  [w], [Write (save)],
  [r], [Re-render the screen],
  [Ctrl-c], [Quit without save],
)
