# Issues
* use after free in some formulas, irregular: I cant reproduce
* After row or column deletion some formulas may break. [Link](#err1).
* Input/edit text on the last cell fails

# To-do
* Toml subset parser
* Improve update screen when it isn't needed to update the full screen.
* On color() formula delete it should clear color.

# vspl issues
[vispel] >> a = 0
0
[vispel] >> a
0
[vispel] >> hh
Var `hh` not declared
[vispel] >> a
3

* print without parenthesis nor arguments segfaults

# vspl todos
* Check that all memleaks are fixed
* Try to find segfaults on weird stuff
* Add global variables as filename, file extension, ... So at can be used to set
  conditional values.
* Only first non declared var is reported

# To-do (maybe)
* Add more movement stuff
* Improve UI and UI customization.
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
* Options
* Formulas and deletion are not friends -> need testing

### err1
After delete a row or a column, ALL formulas that use cells that are moved
should update the id reference to the new one. Formulas that point to a cell
that was deleted should be set to error. 
