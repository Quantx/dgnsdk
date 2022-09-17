# Minimal C Compiler stage 0

## Overview
This program accepts a preprocessed ASCII C source code file through STDIN and preforms one of two operations depending on the arguments provided. If no arguments are provided then the ASCII file is then parsed into a sequence of single byte tokens which are emitted through STDOUT. Any error in parsing will result in the program exiting with status code 1 and printing an error message to STDERR. Alternatively exactly 2 arguments can be provided with the first being an ASCII encoded octal value and the second being a message to display. The octal value is treated as an offset (in tokens) into the provided ASCII C file. The line of the file on which the offset resides is printed along with the provided message which is used to indicate the location in which an error occurred.

## Operation
The program begins by reading in one line of text from the input file (lines are terminated by the ASCII new-line character). The line must contain fewer characters (including the new-line) than the `MAX_LINE` constant. This constant can be easily adjusted in the header file if longer lines need to be processed. Once a line has been read the first character of that line is examined.

### Named Symbols & Reserved Words
If the character is a letter or underscore than the following token must either be a named symbol (variable or function) or a reserved word. Each subsequent character is examined to determine if it is part of the named symbol or some other token.

The entire named symbol is then checked against a list of reserved words and if a match is found, then the corresponding token for that particular reserved word is emitted.

If no match is found, then the named symbol must be user generated and the named symbol token is emitted. The token is then succeed by a 16-bit integer containing the number of characters belonging to the named symbol. These characters are then emitted as a raw ASCII string (this string is NOT null terminated).

### Numerical Constants
If the character is a number then the following token is one of three different types of numerical constants. If the character is not zero then this is a decimal constant. Otherwise, an additional character must be read to determine the type of constant. If this additional character forms the string `0x` then a hexadecimal constant will follow. If the additional character forms the string `0b` then a binary constant follows. Otherwise, it is assumed that an octal constant follows.

Regardless of the type of constant, the input ASCII characters will be processed into an 32-bit integer using the appropriate radix.

If the value of the constant exceeds the value `0xFFFF` or the constant is succeeded by the character `l` then a `LongNumber` token will be generated followed by 32-bit integer value itself.

If the value of the constant is less than the value `0xFF` or the constant is succeeded by the character `c` then a `SmolNumber` token will be generated followed a 16-bit integer containing the least significant 16 bits of the constant.

Otherwise, a `Number` token is generated followed by a 16-bit integer containing the least significant 16 bits of the constant.

### Comments

Upon encountering a forward slash `/` character the subsequent character is also checked.

If a second forward slash is encountered, then the rest of the line is discarded and parsing continues with the next line.

If an asterisk `*` character is encountered then all subsequent input is discarded until the first instance of the following two character terminator sequence is encountered `*/` which is then also discarded. If no terminator sequence is found then an error is thrown, otherwise parsing continues as normal from here.

### Character Constants

A single quotation mark character denotes the beginning of a character constant. A subsequent character is immediately processed to determine the type of constant present.

If the subsequent character is not a backward slash `\` character then a `SmolNumber` token is generated followed by the 16-bit integer value corresponding to the ASCII value of the subsequent character.

If the subsequent character is a backward slash `\` character then an escape sequence is present and one more character must be read to determine which sequence is present. If the character is one of the following then `abefnrtv\'"` the [appropriate sequence](https://en.cppreference.com/w/c/language/escape) is generated. If the character is a number then a one to three digit octal constant is parsed. If the character is `x` then a hexadecimal constant is parsed.

The closing single quotation mark is then parsed and a `SmolNumber` token is generated followed by the appropriate 16-bit integer value as specified by either the specific escape sequence or provided constant.

### String Constants

A double quotation mark character denotes the beginning of a string constant. A `String` token is immediately emitted followed by the raw ASCII contents of the string. This continues until a matching quotation mark which is not proceeded by a backslash character is found. The matching quotation mark is proceesed, but not emitted and parsing continues from here.

Any escape sequences present in the string (other than `\"`) are NOT processed by the compiler and are passed verbatim to the assembler later on.

### Generic Symbols

All other tokens are generic symbols found in C such as `+ - * & | << >>` for example. These are processed trivially and a corresponding token is emitted.

No distinction is made at this time between the prefix and postfix version of the following operators `++` and `--`