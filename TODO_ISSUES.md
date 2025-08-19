# Issues
* use after free in some formulas, irregular: I cant reproduce
* a formula without closing parenthesis falls into an inf loop (I think this is
  not correct but it exits and should set cell as error 

# To-do
* Improve UI and UI customization.
* Improve colors.
* Add color to input.
* Improve input: handle special chars other than BS and Cr.
* Maybe move from ast-walking to VM in formulas.
* Improve update screen when it isn't needed to update the full screen.
* Function as function argument 's name appears between `'`s. 

# No tested at all
* CSV reader.
* Behaviour of corner cases in formulas for builtin, ranges and comparison.
* Possible leak at return Error from invalid stuff
