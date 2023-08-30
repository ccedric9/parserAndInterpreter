# Extension to Lisp Grammar

I have extended the existing Lisp grammar to include additional features. Below are the details of the extensions:

## 1. Operation Function Extension

I've extended the operation function to incorporate more variety in operations. The `<OPERATION_FUNC>` now supports the following grammar:
Where:

- "MUL" stands for multiplication
- "DIV" stands for division
- "MOD" stands for modulus
- "ABS" stands for absolute value

Only atomic Lisp elements can be involved in these operations. Note that while the parser will not check if division by zero occurs, the interpreter will perform this check.

## 2. List Function Extension

The list function has been extended to support variable swaps and concatenation. The grammar for `<LISTFUNC>` is now:

Where:

- "CAT" performs concatenation
- "SWAP" can be used to swap the values and status of two variables

For example, `(SET A '1') (SWAP A B)` will swap the values of A and B.

## 3. IO Function Extension

The IO function now includes a "RESET" function that can reset a defined variable to its default value. The grammar for `<IOFUNC>` is:

For instance, `(RESET A)` would reset the variable A to its default value.

## 4. Boolean Function Extension

The boolean function now includes an "ATOM" function for checking if an element is atomic Lisp. The grammar for `<BOOLFUNC>` is:

Examples include `(IF (ATOM B))` and `(WHILE (ATOM NIL))`, which check if B and NIL are atomic Lisp elements.

These extensions aim to enrich the functionality of the Lisp grammar, allowing for a wider range of operations and capabilities.
