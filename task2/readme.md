# Project README

This project includes the `main.cpp` file, which demonstrates the usage of a thread pool and dynamic loading of a shared library (`libCodec.so`). The program reads input data from a file or standard input, applies encryption or decryption based on the provided key, and writes the result to an output file or standard output.

## Prerequisites

To compile and run this project, you need:

- C++ compiler supporting C++11 or later
- `libCodec.so` shared library (should be located in the same directory as the executable)
- POSIX-compliant operating system (e.g., Linux)

## Compilation

To compile the project, follow these steps:

1. Open a terminal and navigate to the project directory.
2. Execute the following command to compile the code:

```bash
   make 
   ```
This will generate an executable named "coder"

## Usage

The program expects 2 command line arguments: the encryption key and flag.
```bash
./coder key flag < input_file.txt > out_file.txt
```

## Examples:
1. 
```bash
./coder 5 -e < input_file.txt > output_file.txt
```
2. 
```bash
cat file | ./coder 5 -e > output_file.txt 
```

