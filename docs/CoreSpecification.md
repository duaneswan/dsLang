# dsLang Language Core Specification

This document provides a complete, plain-English specification of dsLang's core constructs, focusing on language definitions.

## 1. Variables

Define a variable by using the `define variable` keywords, followed by the variable name, the keyword `as`, and the type name. Variables must be explicitly typed.

Define variable username as String to hold text data.

Define variable isReady as Bool to represent a true/false flag.

Define variable count as Int to store a numeric value.

Variables can be initialized at declaration time by adding an equals sign and the initial value.

Define variable maxSize as Int = 100 to set the initial maximum size.

Define variable enabled as Bool = true to set the initial state to enabled.

Variable names must start with a lowercase letter, followed by any combination of letters, numbers, and underscores.

## 2. Constants

Define a compile-time constant using the `define constant` keywords, followed by the constant name, the keyword `as`, the type name, and the required value.

Define constant MAX_USERS as Int = 100 to set the limit of users.

Define constant VERSION as String = "1.0.0" to specify the version number.

Define constant PI as Float = 3.14159 to define a mathematical constant.

Constants must be initialized at declaration time and cannot be changed after definition. Constant names are conventionally written in uppercase with words separated by underscores.

## 3. Functions

Define a function using the `define function` keywords, followed by the function name, the parameter list, and the return type. Function bodies are enclosed between the declaration and the `end` keyword.

To declare a function named login that takes two parameters:

Define function login with parameters username of type String and password of type String, and it returns Bool indicating success or failure.
    // Function body goes here
    // Statements to perform login logic
    return true or false based on validation
end

To declare a function with no parameters and no return value:

Define function initialize with no parameters and it returns Void.
    // Function body goes here
end

To declare a function with multiple parameters:

Define function calculateTotal with parameters price of type Float, quantity of type Int, and taxRate of type Float, and it returns Float.
    // Function body goes here
    return the calculated total as a Float
end

Function names must start with a lowercase letter, followed by any combination of letters, numbers, and underscores.

## 4. Records (Structs)

Define a record type using the `define record` keywords, followed by the record name. Record definitions are enclosed between the declaration and the `end` keyword.

Define record UserProfile containing fields:
    define userId as Int
    define name as String
    define active as Bool
end

Define record Point containing fields:
    define x as Float
    define y as Float
end

Record names must start with an uppercase letter, followed by any combination of letters, numbers, and underscores. Fields within a record are defined using the same syntax as regular variables.

To access record fields, use dot notation:

Define variable user as UserProfile
user.userId = 1
user.name = "John"
user.active = true

## 5. Protocols (Interfaces)

Define a protocol using the `define protocol` keywords, followed by the protocol name. Protocol definitions are enclosed between the declaration and the `end` keyword.

Define protocol Authenticatable requiring functions:
    define function authenticate with parameters token of type String returns Bool
    define function logout with no parameters returns Void
end

Define protocol Drawable requiring functions:
    define function draw with parameters context of type DrawContext returns Void
    define function getBounds returns Rectangle
end

Protocol names must start with an uppercase letter, followed by any combination of letters, numbers, and underscores. Protocol function declarations do not include function bodies.

To implement a protocol in a record, use the `implements` keyword:

Define record User implements Authenticatable containing fields:
    define userId as Int
    define name as String
    
    define function authenticate with parameters token of type String returns Bool
        // Implementation details
        return true
    end
    
    define function logout with no parameters returns Void
        // Implementation details
    end
end

## 6. Modules

Define a module using the `define module` keywords, followed by the module name. Module definitions are enclosed between the declaration and the `end` keyword.

Define module Auth containing:
    // Records, functions, and other definitions related to authentication
    
    Define record Credentials containing fields:
        define username as String
        define passwordHash as String
    end
    
    Define function validateCredentials with parameters creds of type Credentials returns Bool
        // Implementation
    end
end

To use definitions from a module in another file, import the module:

Import module Auth

Then access the module's definitions using dot notation:

Define variable creds as Auth.Credentials
Define variable isValid as Bool = Auth.validateCredentials(creds)

Module names must start with an uppercase letter, followed by any combination of letters, numbers, and underscores.

## 7. Control Structures

### If-Then-Else

To perform a conditional action, use if-then-else:

If condition then
    // Statements to execute when condition is true
else
    // Statements to execute when condition is false
end

Multiple conditions can be checked using else if:

If condition1 then
    // Statements to execute when condition1 is true
else if condition2 then
    // Statements to execute when condition1 is false and condition2 is true
else
    // Statements to execute when both conditions are false
end

### Loops

To repeat while a condition holds, use a while loop:

Loop while condition
    // Statements to repeat as long as condition is true
end

To iterate over a collection, use a for-in loop:

Loop for item in collection
    // Statements to execute for each item in the collection
end

To iterate with a counter:

Loop for i from 0 to 10
    // Statements to execute for i = 0, 1, 2, ..., 10
end

## 8. Summary of Syntax Rules

- Variables start with lowercase letters (e.g., userName, isActive, count).
- Types, Records, Protocols, and Modules begin with uppercase letters (e.g., String, UserProfile, Authenticatable, Auth).
- Constants are conventionally written in uppercase with underscores (e.g., MAX_SIZE, DEFAULT_VALUE).
- Function names start with lowercase letters (e.g., calculateTotal, validateUser).
- Keywords are all lowercase (e.g., define, as, with, parameters, returns).
- Blocks are delimited with English keywords like "define" at the start and "end" to close.
- No braces or semicolons are used; use indentation and the "end" keyword to close blocks.
- Each definition or statement is on its own line.
- Every variable and function must have an explicit type declaration.
- Parameter lists use the phrase "with parameters" followed by name-type pairs.
- Return types are specified with "returns" followed by the type name.
- Comments are introduced by "//".
- Multi-line comments use "/*" and "*/".
