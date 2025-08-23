# Issues
* use after free in some formulas, irregular: I cant reproduce
* After row or column deletion some formulas may break. [Link](#err1).

# To-do
* Improve update screen when it isn't needed to update the full screen.
* Implement the $ to select what to expand
* On color() formula delete it should clear color.
* Add autosave
* Add config file

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
* Rewrite convert and how to add/update data to cells.

# No tested at all
* CSV reader.
* Behaviour of corner cases in formulas for builtin, ranges and comparison.
* Possible leak at return Error from invalid stuff
* Deletion

### err1
After delete a row or a column, ALL formulas that use cells that are moved
should update the id reference to the new one. Formulas that point to a cell
that was deleted should be set to error. 
