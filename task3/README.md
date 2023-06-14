# stree

`stree` is a command-line program written in C that displays the directory tree structure along with file information such as permissions, user, group, and file size. It provides a visual representation of the directory hierarchy, making it easier to navigate and understand the structure of a directory.

## Usage

To compile the program, use the provided Makefile:
```
make
```

To run the program, execute the compiled binary:
```
./stree
```
By default, it will display the directory tree structure starting from the current working directory.

Alternatively, you can specify a different directory path as a command-line argument:
```
./stree /path/to/directory
```
The program will recursively traverse the directory structure and print the tree representation along with file information.

To remove the executable and all files created, run :
```
make clean
```
## Features

- Displays the directory tree structure with proper indentation.
- Prints file permissions (read, write, execute) for each file.
- Shows the user and group associated with each file.
- Displays the file size in bytes.

Please note the program's limitations, as described in the source code comments.

For more details, please refer to the source code and the provided Makefile.

