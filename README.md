# zenkai
Universal Sudoku solver with a custom-rule engine

***

## About
`zenkai` is a universal `sudoku solver` with a `custom-rule engine`. This means `zenkai` can most likely solve any sudoku you can think of, even with rules beyond the standard sudoku rules.

### Sudoku files
`zenkai` expects sudokus to be defined in a `json` file. The root object MUST HAVE a field `name` with its value being the sudoku grid string (a 81 character string, where the i-th character encodes the i-th digit in the sudoku, 0 encodes an empty cell).
Furthermore, the root object MAY HAVE a `rules` field with its value being an array of objects. Each of those objects encodes an instance of a custom sudoku rule (e.g. a thermometer) by specifying a `name` field with the value being the name of the custom rule and optionally (if needed) a `data` field which can be a generic json object. This `data` object will be translated by `zenkai` from `json` to `lua`. The behaviour for said custom rules are defined by `<name>.lua` script files inside `zenkai`'s `rules` directory.
For more details on how behaviour for custom rules can be defined, check the `rules` subdirectory in this repository.

***

## Installing
Run the following command inside the root directory of this repository:
```bash
make install
```
This will install the `zenkai` binary and the `rule` scripts, found in the `rules` subdirectory.

***

## Usage
```bash
zenkai <sudoku>...
```
where sudoku is a filepath to a `json` file that encodes the sudoku alongside the custom rules for the sudoku, if needed.
