# Issues
* use after free in some formulas, irregular: I cant reproduce

# To-do
* Improve UI and UI customization.
* Improve colors.
* Add color to input.
* Improve input: handle special chars other than BS and Cr.
* Maybe move from ast-walking to VM in formulas.
* Improve update screen when it isn't needed to update the full screen.
* I think that formula error should return Error as value but should keep
  formula as input for being edited.

# No tested at all
* CSV reader.
* Behaviour of corner cases in formulas for builtin, ranges and comparison.
* Possible leak at return Error from invalid stuff
