# Issues
* use after free in some formulas, irregular: I cant reproduce
* a formula without closing parenthesis falls into an inf loop (I think this is
  not correct but it exits and should set cell as error 

# Todo
* Improve UI and UI customization
* Colors?
* Add color to input
* Improve input: handle special chars other than bs and cr
* maybe move from ast-walking to vm in formulas
* improve update screen when it isn't needed to update the full screen

# No tested at all
* csv reader
* behaviour of corner cases in formulas for builtin, ranges and comparison.
* possible leak at return Error from invalid stuff
