# dsLang Language Specification

## Introduction

dsLang is a simple yet powerful programming language designed specifically for building operating systems. It features a C/Objective-C inspired syntax while remaining minimal and focused on low-level systems programming. This document defines the syntax, semantics, and grammar of dsLang.

## Design Goals

* **Simplicity**: Easy to learn and use, with a small core language
* **Low-level Control**: Direct memory access and hardware control for OS development
* **Expressive**: Powerful enough to implement a complete operating system
* **Minimal Runtime**: No runtime library dependencies outside what is explicitly linked
* **Safety**: Provide reasonable compile-time checks while allowing low-level operations when needed

## Lexical Structure

### Comments
```
// Single-line comment

/* Multi-line 
   comment */
```

### Identifiers

Identifiers in dsLang consist of letters, digits, and underscores. They must begin with a letter or underscore.

```
identifier ::= [a-zA-Z_][a-zA-Z0-9_]*
```

### Keywords

The following are reserved keywords in dsLang:

```
if        else      while     for       do        return
break     continue  struct    enum      int       char
bool      long      short     unsigned  void      const
extern    static
```

### Literals

#### Integer Literals
```
integer_literal ::= decimal_literal | hex_literal
decimal_literal ::= [0-9]+
hex_literal     ::= 0[xX][0-9a-fA-F]+
```

#### Character Literals
```
char_literal ::= "'" char "'"
```

#### String Literals
```
string_literal ::= '"' string_content '"'
```

#### Boolean Literals
```
bool_literal ::= "true" | "false"
```

#### Null Literal
```
null_literal ::= "null"
```

## Data Types

### Basic Types
- `void`: Represents no value
- `bool`: Boolean type, can be `true` or `false`
- `char`: 8-bit character type
- `int`: 32-bit signed integer
- `long`: 64-bit signed integer
- `short`: 16-bit signed integer
- `unsigned int`: 32-bit unsigned integer
- `unsigned long`: 64-bit unsigned integer
- `unsigned char`: 8-bit unsigned integer
- `unsigned short`: 16-bit unsigned integer

### Derived Types
- **Pointers**: `type*` (e.g., `int*`, `char*`)
- **Arrays**: `type[size]` (e.g., `int[10]`, `char[64]`)
- **Structures**: User-defined composite types

## Expressions

### Primary Expressions
- Literals (integers, characters, strings, booleans, null)
- Identifiers (variable/function names)
- Parenthesized expressions: `(expression)`

### Arithmetic Operators
- Addition: `+`
- Subtraction: `-`
- Multiplication: `*`
- Division: `/`
- Modulo: `%`

### Increment and Decrement
- Pre-increment: `++expr`
- Pre-decrement: `--expr`
- Post-increment: `expr++`
- Post-decrement: `expr--`

### Bitwise Operators
- AND: `&`
- OR: `|`
- XOR: `^`
- NOT: `~`
- Left shift: `<<`
- Right shift: `>>`

### Logical Operators
- AND: `&&`
- OR: `||`
- NOT: `!`

### Comparison Operators
- Equal: `==`
- Not equal: `!=`
- Less than: `<`
- Greater than: `>`
- Less than or equal: `<=`
- Greater than or equal: `>=`

### Assignment Operators
- Basic assignment: `=`
- Compound assignments: `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, `^=`, `<<=`, `>>=`

### Member Access
- Structure field access: `struct.field`
- Pointer field access: `ptr->field`

### Array Subscripting
- Array indexing: `array[index]`

### Function Calls
- Regular call: `function(arg1, arg2, ...)`

### Objective-C Style Message Send
- Message send: `[receiver message]`
- With parameters: `[receiver message:arg1]`
- Multiple parameters: `[receiver message:arg1 secondParam:arg2]`

### Type Casting
- Cast operator: `(type)expr`

### Operator Precedence

Operators are listed in descending order of precedence:

1. `()` `[]` `.` `->`
2. `!` `~` `++` `--` `(type)` `*` (dereference) `&` (address-of)
3. `*` `/` `%`
4. `+` `-`
5. `<<` `>>`
6. `<` `<=` `>` `>=`
7. `==` `!=`
8. `&`
9. `^`
10. `|`
11. `&&`
12. `||`
13. `=` `+=` `-=` `*=` `/=` `%=` `&=` `|=` `^=` `<<=` `>>=`

## Statements

### Expression Statement
```
expr_stmt ::= expression ";"
```

### Compound Statement (Block)
```
compound_stmt ::= "{" statement* "}"
```

### Conditional Statement
```
if_stmt ::= "if" "(" expression ")" statement [ "else" statement ]
```

### Loop Statements
```
while_stmt ::= "while" "(" expression ")" statement
do_while_stmt ::= "do" statement "while" "(" expression ")" ";"
for_stmt ::= "for" "(" [expression | declaration] ";" [expression] ";" [expression] ")" statement
```

### Jump Statements
```
return_stmt ::= "return" [expression] ";"
break_stmt ::= "break" ";"
continue_stmt ::= "continue" ";"
```

### Declaration Statement
```
decl_stmt ::= declaration ";"
```

## Declarations

### Variable Declaration
```
var_decl ::= type_specifier declarator ["=" initializer] ";"
```

### Function Declaration
```
func_decl ::= type_specifier identifier "(" parameter_list ")" compound_stmt
parameter_list ::= [parameter ("," parameter)*]
parameter ::= type_specifier identifier
```

### Structure Declaration
```
struct_decl ::= "struct" identifier "{" field_decl* "}" ";"
field_decl ::= type_specifier identifier ";" 
```

### Enum Declaration
```
enum_decl ::= "enum" identifier "{" enum_value ("," enum_value)* "}" ";"
enum_value ::= identifier ["=" expression]
```

## Type Specifiers
```
type_specifier ::= ["unsigned"] ("void" | "bool" | "char" | "int" | "long" | "short") 
                  | "struct" identifier
                  | "enum" identifier
                  | type_specifier ("*" | "[" [integer_literal] "]")
```

## Complete EBNF Grammar

```ebnf
program ::= declaration*

declaration ::= var_decl | func_decl | struct_decl | enum_decl

var_decl ::= type_specifier declarator ["=" initializer] ";"
declarator ::= identifier | array_declarator
array_declarator ::= identifier "[" [expression] "]"

func_decl ::= type_specifier identifier "(" parameter_list ")" compound_stmt
parameter_list ::= [parameter ("," parameter)*]
parameter ::= type_specifier identifier

struct_decl ::= "struct" identifier "{" field_decl* "}" ";"
field_decl ::= type_specifier identifier ";" 

enum_decl ::= "enum" identifier "{" enum_value ("," enum_value)* "}" ";"
enum_value ::= identifier ["=" expression]

type_specifier ::= ["unsigned"] ("void" | "bool" | "char" | "int" | "long" | "short") 
                  | "struct" identifier
                  | "enum" identifier
                  | type_specifier ("*" | "[" [integer_literal] "]")

statement ::= expr_stmt
           | compound_stmt
           | if_stmt
           | while_stmt
           | do_while_stmt
           | for_stmt
           | return_stmt
           | break_stmt
           | continue_stmt
           | decl_stmt

expr_stmt ::= expression ";"
compound_stmt ::= "{" statement* "}"
if_stmt ::= "if" "(" expression ")" statement [ "else" statement ]
while_stmt ::= "while" "(" expression ")" statement
do_while_stmt ::= "do" statement "while" "(" expression ")" ";"
for_stmt ::= "for" "(" [expression | declaration] ";" [expression] ";" [expression] ")" statement
return_stmt ::= "return" [expression] ";"
break_stmt ::= "break" ";"
continue_stmt ::= "continue" ";"
decl_stmt ::= declaration ";"

expression ::= assignment_expr

assignment_expr ::= logical_or_expr (assignment_op assignment_expr)?
assignment_op ::= "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "&=" | "|=" | "^=" | "<<=" | ">>="

logical_or_expr ::= logical_and_expr ("||" logical_and_expr)*
logical_and_expr ::= bitwise_or_expr ("&&" bitwise_or_expr)*
bitwise_or_expr ::= bitwise_xor_expr ("|" bitwise_xor_expr)*
bitwise_xor_expr ::= bitwise_and_expr ("^" bitwise_and_expr)*
bitwise_and_expr ::= equality_expr ("&" equality_expr)*
equality_expr ::= relational_expr (("==" | "!=") relational_expr)*
relational_expr ::= shift_expr (("<" | ">" | "<=" | ">=") shift_expr)*
shift_expr ::= additive_expr (("<<" | ">>") additive_expr)*
additive_expr ::= multiplicative_expr (("+" | "-") multiplicative_expr)*
multiplicative_expr ::= unary_expr (("*" | "/" | "%") unary_expr)*

unary_expr ::= postfix_expr
            | "++" unary_expr
            | "--" unary_expr
            | unary_op unary_expr
            | "(" type_specifier ")" unary_expr  /* Type cast */
unary_op ::= "+" | "-" | "!" | "~" | "*" | "&"

postfix_expr ::= primary_expr
               | postfix_expr "[" expression "]"
               | postfix_expr "(" [arg_list] ")"
               | postfix_expr "." identifier
               | postfix_expr "->" identifier
               | postfix_expr "++"
               | postfix_expr "--"
               | "[" expression message_expr "]"  /* Objective-C style message */

message_expr ::= identifier
               | identifier ":" expression
               | identifier ":" expression identifier ":" expression

arg_list ::= assignment_expr ("," assignment_expr)*

primary_expr ::= identifier
               | literal
               | "(" expression ")"

literal ::= integer_literal
          | char_literal
          | string_literal
          | bool_literal
          | null_literal

integer_literal ::= decimal_literal | hex_literal
decimal_literal ::= [0-9]+
hex_literal ::= 0[xX][0-9a-fA-F]+
char_literal ::= "'" char "'"
string_literal ::= '"' string_content '"'
bool_literal ::= "true" | "false"
null_literal ::= "null"
```

## Type System

### Type Compatibility

- Integer types can be implicitly converted between different sizes, but signed-to-unsigned conversions require explicit casts.
- Pointers to different types are incompatible and require explicit casts.
- `void*` can be implicitly converted to any pointer type and vice versa.
- Structures are only compatible if they are the same type (name-based compatibility).
- Arrays can decay into pointers to their first element.

## Memory Management

dsLang does not include garbage collection or automatic memory management. Memory is manually managed through the standard library functions `malloc` and `free`.

## Standard Library

Core functionalities are provided in the standard library, including:

- Memory management functions: `malloc`, `free`, `memset`, `memcpy`, etc.
- String manipulation: `strlen`, `strcpy`, `strcmp`, etc.
- Console I/O: `putchar`, `puts`, `printf`, etc.
- Hardware access functions: `inb`, `outb`, etc. (for port I/O)

## Objective-C Style Message Syntax

dsLang includes a simplified version of Objective-C's message syntax:

```
// Basic message (no parameters)
[object method]

// Single parameter method
[object method:value]

// Multiple parameter method
[object method:value1 param2:value2 param3:value3]
```

Unlike Objective-C, dsLang's message syntax is just syntactic sugar for function calls. At compile time, messages are converted to function calls, with the receiver as the first argument:

```
// This dsLang code
[obj doSomething];

// Becomes equivalent to
doSomething(obj);

// And this dsLang code
[obj setValue:42 withName:"answer"];

// Becomes equivalent to 
setValue_withName(obj, 42, "answer");
```

This approach provides an Objective-C like syntax without needing a complex runtime dispatch system.
