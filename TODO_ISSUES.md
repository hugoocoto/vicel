# Issues
* use after free in some formulas(at unsubscribe), irregular: I cant reproduce

# To-do
* After row or column deletion some formulas may break. [Link](#err1).
* Improve update screen when it isn't needed to update the full screen.
* On color() formula deletion it should clear color.
* Add history (undo redo)
* Support more than ASCII chars.

# To-do (maybe)
* Add more movement stuff
* Improve UI and UI customization.
* Improve input: handle special chars other than BS and Cr. 
* Maybe move from ast-walking to VM in formulas.
* Maybe add date/time types
* Add copy for blocks
* Rewrite convert and how to add/update data to cells.
* Add the option --serve and --connect and let connect to a external vicel
  session. It would be awesome but it's a bug mine (too much work).
* Let open/edit new files and save current with other name.
* Improve mappings customization 

# No tested at all
* CSV reader.
* Behaviour of corner cases in formulas for builtin, ranges and comparison.
* Deletion
* Formulas and deletion are not friends -> need testing

### err1
After delete a row or a column, ALL formulas that use cells that are moved
should update the id reference to the new one. Formulas that point to a cell
that was deleted should be set to error. 

Good news! As it stores cells by reference, the subscriptions are correct. It's
only needed to change formula text. 

# Things todo if for somewhat reason it have to become useful
* Improve formulas: move from ast-walking to something more performant. 
* Fix all previously written issues and todos.

