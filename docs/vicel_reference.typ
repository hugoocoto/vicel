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
have it installed in #smallcaps("~/.local/bin") then you can start it as a
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
  [`-m`, `--use-mouse`  ], [Enable mouse support],
  [`-D`, `--debug`      ], [Enable debug output ],
  [`-c`, `--config-file`], [Set custom file path],
),
