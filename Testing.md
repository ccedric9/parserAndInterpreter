# Assignment Testing and Code Coverage

## Goal of Testing

The primary goals of testing for this assignment are as follows, ranked in order of priority:

1. **Accuracy**: Ensuring the correctness and accuracy of the code.
2. **Code Coverage**: Increasing the code coverage as much as possible to test different paths.
3. **Readability and Maintainability**: Ensuring the code is readable and maintainable for future modifications.

## Testing Process

### Unit Tests

The `tests()` function is designed to perform unit tests for each function in the code. It covers various test cases to secure accuracy and increase code coverage.

### Reading Source Inputs

The `testReadFile()` function, found within `tests()`, reads random source inputs to ensure that the sources are correctly read and parsed.

### Running Code Coverage Report

There's an additional test function called `readMultipleFiles()`. This function is designed to cover a sufficient number of test cases for generating the CCR. To avoid confusion, it is currently commented out, as it might generate "Parse OK" outputs.

## CCR (Code Coverage Report)

The CCR indicates the percentage of lines executed during testing for both the parser and the interpreter.

### Parser Coverage Report

- File 'nuclei.c'
- Lines executed: 86.36% of 623

In the parser's CCR, you'll notice that there are lines (totalling 85) that cannot be covered due to EXIT_FAILURE (on_error) or bool false conditions. These cases can't be asserted because they would lead to code halting in false/on_error scenarios.

### Interpreter Coverage Report

- File 'nuclei.c'
- Lines executed: 87.87% of 874

Similar to the parser, the interpreter's CCR indicates 106 lines that can't be covered due to EXIT_FAILURE (on_error) or bool false conditions.

Please note that the `testReadFile()` and `readMultipleFiles()` functions are commented out to avoid confusion and unexpected outputs.

## Generating CCR

To generate the CCR for the assignment:

1. **Clean Up**: Run `make clean` to remove any previous reports.
2. **Compile Code**: Run `make all` to compile the code.
3. **Parser CCR**: Run `./parse_c (any source file).ncl` to test the parser. Use any of the test cases included in `tests()`.
4. **Interpreter CCR** (Optional): Run `make interp_c` to compile the interpreter (if required). Run `./interp_c (any source file).ncl` to test the interpreter.
5. **Generate Report**: After running tests, execute `gcov nuclei.c` to generate the CCR.

Please note that `testReadFile()` and `readMultipleFiles()` are designed for thorough testing but are currently commented to avoid unexpected outputs.
