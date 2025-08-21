# Issues
* use after free in some formulas, irregular: I cant reproduce

# To-do
* Add color to input.
* Improve update screen when it isn't needed to update the full screen.
* I think that formula error should return Error as value but should keep
  formula as input for being edited.
* Implement the $ to select what to expand
* Maybe add date/time types
* Try to expand with the difference between cell and the one in the opposite
  direction at least for numbers, that way if the two first numbers are written
  at the time o expansion it would use the desired spacing.
* Add literal() builtin that evaluates to the value inside, to try to fix set
  color from color() in a different cell.
* I think that if a formula is at the last right cell it breaks on each comma.
  Untested but sometimes fails. Idk if I fix it yet.

# To-do (maybe)
* Add more movement stuff
* Improve UI and UI customization.
* Improve colors.
* Improve input: handle special chars other than BS and Cr.
* Maybe move from ast-walking to VM in formulas.

# No tested at all
* CSV reader.
* Behaviour of corner cases in formulas for builtin, ranges and comparison.
* Possible leak at return Error from invalid stuff

