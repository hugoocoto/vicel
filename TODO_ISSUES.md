# Issues
* use after free in some formulas, irregular: I cant reproduce

# To-do
* Add inset row at the left, right (same for columns)
* Add delete rows and columns
* Add color to input.
* Improve update screen when it isn't needed to update the full screen.
* Implement the $ to select what to expand
* On color() formula delete it should clear color.
* Rewrite convert and how to add/update data to cells.

# To-do (maybe)
* Add more movement stuff
* Improve UI and UI customization.
* Improve colors.
* Improve input: handle special chars other than BS and Cr.
* Maybe move from ast-walking to VM in formulas.
* Maybe add date/time types
* Try to expand with the difference between cell and the one in the opposite
  direction at least for numbers, that way if the two first numbers are written
  at the time of expansion it would use the desired spacing.
* Add copy for blocks

# No tested at all
* CSV reader.
* Behaviour of corner cases in formulas for builtin, ranges and comparison.
* Possible leak at return Error from invalid stuff

