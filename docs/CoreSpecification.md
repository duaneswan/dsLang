# dsLang Core Language Specification

This document provides a comprehensive specification of dsLang's core constructs, written in plain English. It is designed to guide new users through the language's fundamental features and syntax.

## 1. Variables

Variables in dsLang are used to store data values that can be modified during program execution. To declare a variable, you must specify both its name and type.

Define variable by writing the type name followed by the variable name. For example:

Define variable username as String to store user identification text.
Define variable count as Int to maintain a numeric counter.
Define variable isActive as Bool to represent a true or false state.
Define variable dataBuffer as Char array to hold a sequence of characters.

Multiple variables of the same type can be declared on a single line by separating the variable names with commas. For example:

Define variables firstName, lastName as String to store user name components.
Define variables width, height, depth as Int to store dimensional measurements.

Variables must be declared before they can be used, and they can be assigned initial values during declaration:

Define variable maxAttempts as Int with initial value 3 to limit login tries.
Define variable defaultGreeting as String with value "Hello, World!" to provide a standard welcome message.

## 2. Constants

Constants in dsLang are values that cannot be changed after they are initially defined. They are useful for values that should remain fixed throughout program execution.

Define a constant by using the keyword "const" before the type and name, and provide its value at declaration time:

Define constant MAX_USERS as Int with value 100 to set the maximum number of users allowed.
Define constant PI as Double with value 3.14159 to represent the mathematical constant.
Define constant APP_VERSION as String with value "1.0.0" to identify the software version.
Define constant DAYS_IN_WEEK as Int with value 7 to represent the number of days in a week.

Constants must be initialized when they are declared, and any attempt to modify a constant after declaration will result in a compile-time error.

## 3. Functions

Functions in dsLang encapsulate blocks of code that can be called multiple times with different parameters. They allow for code reuse and better organization.

To declare a function, specify its return type, name, and parameters:

Define function calculateArea with parameters length of type Double and width of type Double, and it returns Double representing the area of a rectangle.

The function body begins after the parameter list and continues until the "end" keyword:

Define function login with parameters username of type String and password of type String, and it returns Bool indicating success or failure.
The function body begins here and includes all the code to validate credentials.
The function body ends with the "end" keyword.

Functions that do not return a value use the Void return type:

Define function displayMessage with parameter message of type String, and it returns Void.
The function body begins here and contains the code to output the message.
The function body ends with the "end" keyword.

Function parameters can have default values:

Define function connect with parameters hostname of type String and port of type Int with default value 80, and it returns Bool indicating success or failure.

Functions are called by writing their name followed by parentheses containing any required arguments:

Call the function calculateArea with arguments 5.0 and 3.0 to determine the area.

## 4. Records (Structs)

Records in dsLang are used to group related data fields together into a single composite type. They allow you to create complex data structures that represent real-world entities.

To define a record, use the "record" keyword followed by the record name and its fields:

Define record UserProfile containing:
Field userId of type Int to uniquely identify each user.
Field name of type String to store the user's full name.
Field email of type String to store the user's email address.
Field isActive of type Bool to track whether the user account is active.
End of record UserProfile.

Records can be instantiated by specifying values for their fields:

Create a UserProfile with userId set to 1001, name set to "John Doe", email set to "john@example.com", and isActive set to true.

Fields of a record are accessed using dot notation:

Access the name field of userProfile to retrieve the user's name.
Modify the isActive field of userProfile to false to deactivate the account.

Records can also contain nested records:

Define record Address containing:
Field street of type String.
Field city of type String.
Field country of type String.
End of record Address.

Define record Employee containing:
Field name of type String.
Field homeAddress of type Address.
Field workAddress of type Address.
End of record Employee.

## 5. Protocols (Interfaces)

Protocols in dsLang define a set of methods that a record can implement. They establish a contract that ensures conforming records provide specific functionality.

To define a protocol, use the "protocol" keyword followed by the protocol name and the required methods:

Define protocol Authenticatable requiring:
Function authenticate with parameter token of type String, returning Bool.
Function logout with no parameters, returning Void.
End of protocol Authenticatable.

Records can implement protocols by providing implementations for all required methods:

Define record User implementing Authenticatable containing:
Field username of type String.
Field isLoggedIn of type Bool.

Implementation of function authenticate with parameter token of type String, returning Bool:
Function body begins here with authentication logic.
Function body ends with "end" keyword.

Implementation of function logout with no parameters, returning Void:
Function body begins here with logout logic.
Function body ends with "end" keyword.
End of record User.

Protocols can inherit from other protocols to create more specialized interfaces:

Define protocol SecureAuthenticatable inheriting from Authenticatable requiring:
Additional function verifyTwoFactor with parameter code of type String, returning Bool.
End of protocol SecureAuthenticatable.

## 6. Modules

Modules in dsLang are used to organize code into logical units, making the codebase more maintainable and reusable. They help prevent naming conflicts and provide a way to encapsulate related functionality.

To define a module, use the "module" keyword followed by the module name:

Define module Auth containing:
The module body begins here and includes related records, functions, and protocols.
The module body ends with "end" keyword.
End of module Auth.

To use definitions from a module in another file, you must import it:

Import module Auth to use its authentication functionality.
Import module FileSystem to use file handling capabilities.

You can import specific components from a module:

Import function login from module Auth to use only the login functionality.
Import record UserProfile from module Auth to use only the user profile structure.

Modules can be nested to create hierarchical organization:

Define module Network containing:
Define module HTTP containing:
Define function get with parameter url of type String, returning String.
End of module HTTP.
End of module Network.

To access a nested module's components, use dot notation:

Use Network.HTTP.get with argument "https://example.com" to make an HTTP request.

## 7. Control Structures

Control structures in dsLang allow you to alter the flow of program execution based on conditions or to repeat blocks of code.

**Conditional Execution:**

To perform a conditional action, use "if" followed by a condition:

If condition isLoggedIn is true then
    Display welcome message.
Else
    Prompt user to log in.
End of if statement.

Multiple conditions can be checked using "else if":

If condition score is greater than 90 then
    Assign grade "A".
Else if condition score is greater than 80 then
    Assign grade "B".
Else if condition score is greater than 70 then
    Assign grade "C".
Else
    Assign grade "F".
End of if statement.

**Loops:**

To repeat while a condition holds, use "loop while":

Loop while condition counter is less than 10
    Increment counter.
    Perform action.
End of loop.

To repeat until a condition becomes true, use "loop until":

Loop until condition isDataReceived is true
    Check for incoming data.
    Wait for 100 milliseconds.
End of loop.

To iterate over a range of values, use "loop for":

Loop for variable i from 0 to 9
    Process item at index i.
End of loop.

To iterate with a custom increment, specify the step:

Loop for variable i from 0 to 100 with step 10
    Process item at index i.
End of loop.

**Early Termination:**

To exit a loop early, use "break":

Loop while condition isRunning is true
    Process data.
    If condition errorOccurred is true then
        Break out of the loop.
    End of if statement.
End of loop.

To skip the rest of the current iteration and continue with the next one, use "continue":

Loop for variable i from 0 to 9
    If condition shouldSkip(i) is true then
        Continue to the next iteration.
    End of if statement.
    Process item at index i.
End of loop.

## 8. Summary of Syntax Rules

Here is a summary of dsLang's key syntax rules and conventions:

- Variables start with lowercase letters (e.g., userName, itemCount).
- Types begin with uppercase letters (e.g., Int, String, UserProfile).
- Blocks are delimited by English keywords like "Define" and "End" rather than braces.
- No semicolons are required at the end of statements.
- Indentation is used to improve readability, but it doesn't affect program semantics.
- Keywords like "if", "loop", "define" are used instead of symbols.
- Functions are called by writing their name followed by arguments in parentheses.
- Record fields are accessed using dot notation (record.field).
- Modules and their components are accessed using dot notation (Module.component).
- Conditions in if statements and loops don't require parentheses.
- Comments begin with "//" for single-line comments and "/* ... */" for multi-line comments.
- Strings are enclosed in double quotes ("text").
- Characters are enclosed in single quotes ('a').
- Boolean values are represented as "true" and "false".
- Logical operators use English words: "and", "or", "not".
- Comparison operators include: "is equal to", "is not equal to", "is greater than", "is less than".
