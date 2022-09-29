# Minimal C Compiler stage 1

## Overview
This program accepts a sequence of tokens as emitted by Stage 0 and generates a series of Abstract Syntax Trees which correspond to each statement of the provided input file. Additionally, a number of human readable ASCII segment files `.seg` are generated which contain constant values and allocation information. These will be passed to the assembler later on.

## Theory
Any given C source file consists of a number of namespaces, definitions, and statements.

### Namespaces
A namespace defines a region where definitions are said to be accessible and are arranged in a hierarchical manner.

#### File Namespace
Initially, all definitions must be made in the file namespace. All other namespaces will be children of the file namespace. Any declarations made in the file namespace are treated as globally visible by all subsequent namespace. Only definitions can be made in the file namespace. The file namespace is the only namespace where functions can be declared. Internally, the file namespace is a special type of block namespace.

#### Block Namespace
A block namespace is the most commonly seen and is the only namespace which can contain statements. It can also contain any type of definition.

#### Struct & Union Namespaces
These namespaces are used when defining either a struct or union.

#### Function & Variadic Function Namespaces
These namespaces are used when defining the arguments of a given function. If the last argument defined is the variadic operator then a variadic function namespace is used to denote this.

#### Casting Namespace
This is a special namespace which is used only to process casts. Definitions in this namespace are not visible in any other namespace and are deleted immediately after the statement they were declared in finishes processing.

### Definitions
A definition consists of an optional storage modifier, an optional sign modifier, a data type, and finally one or more declarations.

#### Storage Modifiers
There are five storage modifiers: `extern const register static auto`

The `extern` modifier can be used by itself or may precede any of the other storage types except `static`. The `extern` modifier denotes that the following declarations should be made accessible to other programs by the linker. Following declarations will reside in the data segment unless otherwise specified by `const` or `register`.

The `const` modifier specifies that the following declarations should be stored in the read-only constant segment.

The `register` modifier specifies that the following declarations should be stored in zero-page at all times. This modifier is potentially dangerous and should be used with caution. I might end up either ignoring or removing this entirely, depending on which optimization strategies I end up using.

The `static` modifier specifics that the following declarations should not be made accessible to other programs by the linker. Additionally, it specifies that the following declarations should reside in the data segment.

The `auto` storage type does nothing and is the same as omitting the storage type entirely.

#### Sign Modifier
There are two sign modifiers `signed` and `unsigned`. They must only be used with integer types.

The `signed` modifier does nothing and is the same as omitting the sign modifier entirely. All integer types are assumed to be signed by default.

The `unsigned` modifier specifies that the following integer type should be treated as unsigned in all operations which are affected by sign.

### Primitive Data Types
Primitive types are defined by the compiler. Primitive types consist of both arithmetic and integer types. All primitive types are arithmetic with the exception of `void`.

#### Void
Void is a special primitive type which is used to denote the absence of any particular type and has a storage size of zero bytes. Because this type has no actual storage size it is only to be used indirectly through pointers. Thus, attempting to deference a void pointer to it's direct type will result in an error.

### Char (Character)
Char is the smallest primitive integer type with a storage size of half a word (1 byte). This type is best suited for storing individual ASCII characters. Due to the lack of support for byte operations in some DGN architectures, minor performance penalties may be incurred when operating on chars.

### Int (Integer) (Also Short)
Integers are exactly one word (2 bytes) in size and are thus the fastest primitive integer data types to operate on. While the `short` keyword is supported, internally all uses of this keyword are converted to the `int` type automatically. Thus no distinction between `short` and `int` is made by the compiler.

### Long (Double length Integer)
Longs are two words (4 bytes) in size allowing them to handle much larger values than other integer types at the cost of performance. Because the DGN offers virtually no hardware support for double word operations, long types incur the moderate penalties penalties may be incurred when operating on longs. Additionally, certain operations such as multiplication may be unsupported with this datatype.

### Float (Floating Point)
Floats are also two words (4 bytes) in size and can store a vast range of values at the cost of precision. Please note that the floating point format used by DG is entirely proprietary and in no way compliant with [IEEE 754](https://en.wikipedia.org/wiki/IEEE_754).

In order to utilize the floating point types, a FPU (floating point unit. aka, floating point co-processor) must be present in the system as this type is not natively supported by any DGN system. Alternatively, a floating point library may be used instead to implement any necessary floating point operations in software (assuming one ever gets written).

### Double (Double length Floating Point)
Doubles are a whopping four words (8 bytes) and offer additional precision compared to floats without necessarily incurring performance penalties (at least when an FPU is used). Also not compatible with [IEEE 754](https://en.wikipedia.org/wiki/IEEE_754).

### Structure Data Types
Structure types are defined by the user and can consist of multiple primitive types and other structure types. Please note that internally structure data types are used for both `struct` and `union` types.

### Structs

### Unions