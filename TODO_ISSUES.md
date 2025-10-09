# Issues
* use after free in some formulas(at unsubscribe), irregular: I cant reproduce

# To-do
* After row or column deletion some formulas may break. [Link](#err1).
* Improve update screen when it isn't needed to update the full screen.
* On color() formula deletion it should clear color.
* Input/edit text on the last cell works but it's not what is expected

# To-do (maybe)
* Add more movement stuff
* Improve UI and UI customization.
* Improve input: handle special chars other than BS and Cr.
* Maybe move from ast-walking to VM in formulas.
* Maybe add date/time types
* Try to expand with the difference between cell and the one in the opposite
  direction at least for numbers, so if the two first numbers are written
  at the time of expansion it would use the desired spacing.
* Add copy for blocks
* Rewrite convert and how to add/update data to cells.
* Add the option --serve and --connect and let connect to a external vicel
  session. It would be awesome but it's a bug mine.
* Add history (undo redo)

# No tested at all
* CSV reader.
* Behaviour of corner cases in formulas for builtin, ranges and comparison.
* Deletion
* Formulas and deletion are not friends -> need testing

### err1
After delete a row or a column, ALL formulas that use cells that are moved
should update the id reference to the new one. Formulas that point to a cell
that was deleted should be set to error. 

# Things todo if for somewhat reason it have to become useful
* Improve formulas: move from ast-walking to something more performant. 
* Fix all previously written issues and todos.

