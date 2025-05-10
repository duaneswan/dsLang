/**
 * parser.cpp - Parser Implementation for dsLang
 * 
 * This file implements the Parser class which is responsible for parsing
 * source tokens into an Abstract Syntax Tree (AST).
 */

#include "parser.h"
#include "diagnostic.h"
#include <iostream>
#include <sstream>

namespace dsLang {

/**
 * Constructor - Initialize parser with a lexer
 */
Parser::Parser(Lexer& lexer, DiagnosticReporter& diag_reporter)
    : lexer_(lexer), diag_reporter_(diag_reporter), has_errors_(false) {
    // Prime the parser by fetching the first token
    current_token_ = lexer_.GetNextToken();
}

/**
 * Parse - Parse the source code and build an AST
 */
std::shared_ptr<CompilationUnit> Parser::Parse() {
    return ParseCompilationUnit();
}

//===----------------------------------------------------------------------===//
// Token Utilities
//===----------------------------------------------------------------------===//

/**
 * Consume - Consume the current token if it matches the expected kind
 */
bool Parser::Consume(TokenKind kind, const std::string& error_msg) {
    if (Check(kind)) {
        Advance();
        return true;
    }
    
    ReportError(error_msg);
    return false;
}

/**
 * Match - Check if the current token matches the expected kind and consume it
 */
bool Parser::Match(TokenKind kind) {
    if (Check(kind)) {
        Advance();
        return true;
    }
    return false;
}

/**
 * Advance - Advance to the next token
 */
Token Parser::Advance() {
    Token previous = current_token_;
    current_token_ = lexer_.GetNextToken();
    return previous;
}

/**
 * Peek - Peek at the current token
 */
Token Parser::Peek() const {
    return current_token_;
}

/**
 * PeekNext - Peek at the next token
 */
Token Parser::PeekNext() const {
    return lexer_.PeekNextToken();
}

/**
 * Check - Check if the current token is of the expected kind
 */
bool Parser::Check(TokenKind kind) const {
    if (IsAtEnd()) return false;
    return current_token_.GetKind() == kind;
}

/**
 * CheckNext - Check if the next token is of the expected kind
 */
bool Parser::CheckNext(TokenKind kind) const {
    Token next = lexer_.PeekNextToken();
    return next.GetKind() == kind;
}

/**
 * IsAtEnd - Check if we've reached the end of the token stream
 */
bool Parser::IsAtEnd() const {
    return current_token_.GetKind() == TokenKind::END_OF_FILE;
}

/**
 * ReportError - Report an error at the current token
 */
void Parser::ReportError(const std::string& message) {
    has_errors_ = true;
    diag_reporter_.ReportError(message, current_token_, lexer_.GetFilename());
    
    // Try to recover
    Synchronize();
}

/**
 * Synchronize - Synchronize after an error
 */
void Parser::Synchronize() {
    Advance(); // Skip the token that caused the error
    
    while (!IsAtEnd()) {
        // Stop at statement boundaries
        if (Peek().GetKind() == TokenKind::SEMICOLON) {
            Advance();
            return;
        }
        
        // Or at the start of the next declaration or statement
        switch (Peek().GetKind()) {
            case TokenKind::KW_IF:
            case TokenKind::KW_WHILE:
            case TokenKind::KW_FOR:
            case TokenKind::KW_RETURN:
            case TokenKind::KW_BREAK:
            case TokenKind::KW_CONTINUE:
            case TokenKind::KW_VOID:
            case TokenKind::KW_BOOL:
            case TokenKind::KW_CHAR:
            case TokenKind::KW_SHORT:
            case TokenKind::KW_INT:
            case TokenKind::KW_LONG:
            case TokenKind::KW_FLOAT:
            case TokenKind::KW_DOUBLE:
            case TokenKind::KW_STRUCT:
            case TokenKind::KW_ENUM:
                return;
            default:
                break;
        }
        
        Advance();
    }
}

//===----------------------------------------------------------------------===//
// Declarations
//===----------------------------------------------------------------------===//

/**
 * ParseCompilationUnit - Parse a compilation unit
 */
std::shared_ptr<CompilationUnit> Parser::ParseCompilationUnit() {
    std::vector<std::shared_ptr<Decl>> declarations;
    
    // Parse declarations until we reach the end of the file
    while (!IsAtEnd()) {
        try {
            auto decl = ParseDeclaration();
            if (decl) {
                declarations.push_back(decl);
            }
        } catch (const std::exception& e) {
            // If an exception is thrown during parsing, report an error and continue
            std::string error_msg = "Exception during parsing: ";
            error_msg += e.what();
            ReportError(error_msg);
        }
    }
    
    return std::make_shared<CompilationUnit>(declarations);
}

/**
 * ParseDeclaration - Parse a declaration
 */
std::shared_ptr<Decl> Parser::ParseDeclaration() {
    // Check for struct declaration
    if (Match(TokenKind::KW_STRUCT)) {
        return ParseStructDeclaration();
    }
    
    // Check for enum declaration
    if (Match(TokenKind::KW_ENUM)) {
        return ParseEnumDeclaration();
    }
    
    // Otherwise, must be a function or variable declaration
    // First, parse the type
    auto type = ParseType();
    if (!type) {
        ReportError("Expected type in declaration");
        return nullptr;
    }
    
    // Check for function or variable declaration
    if (Check(TokenKind::IDENTIFIER)) {
        std::string name = Peek().GetLexeme();
        Advance();
        
        // Save the current state for possible backtracking
        Token saved_token = current_token_;
        
        // Check if this is a function declaration
        if (Check(TokenKind::LEFT_PAREN)) {
            return ParseFunctionDeclaration();
        }
        
        // Otherwise, it's a variable declaration
        return ParseVariableDeclaration();
    } else if (Check(TokenKind::LEFT_BRACKET)) {
        // Could be a method declaration (Objective-C style)
        return ParseMethodDeclaration();
    }
    
    ReportError("Expected identifier or method name after type");
    return nullptr;
}

/**
 * ParseFunctionDeclaration - Parse a function declaration
 */
std::shared_ptr<FuncDecl> Parser::ParseFunctionDeclaration() {
    // Get the return type
    auto return_type = ParseType();
    if (!return_type) {
        ReportError("Expected return type for function");
        return nullptr;
    }
    
    // Get the function name
    if (!Check(TokenKind::IDENTIFIER)) {
        ReportError("Expected function name");
        return nullptr;
    }
    
    std::string name = Peek().GetLexeme();
    Advance();
    // Consume the opening parenthesis
    Consume(TokenKind::LEFT_PAREN, "Expected '(' after function name");
    
    // Parse parameters
    std::vector<std::shared_ptr<ParamDecl>> parameters;
    
    if (!Check(TokenKind::RIGHT_PAREN)) {
        do {
            auto param = ParseParameterDeclaration();
            if (param) {
                parameters.push_back(param);
            }
        } while (Match(TokenKind::COMMA));
    }
    
    // Consume the closing parenthesis
    Consume(TokenKind::RIGHT_PAREN, "Expected ')' after function parameters");
    
    // Parse the function body
    std::shared_ptr<BlockStmt> body = nullptr;
    
    if (Match(TokenKind::SEMICOLON)) {
        // Function declaration (no body)
        return std::make_shared<FuncDecl>(name, return_type, parameters, body);
    }
    
    // Function definition (with body)
    body = ParseBlockStatement();
    
    return std::make_shared<FuncDecl>(name, return_type, parameters, body);
}

/**
 * ParseMethodDeclaration - Parse a method declaration (Objective-C style)
 */
std::shared_ptr<MethodDecl> Parser::ParseMethodDeclaration() {
    // Get the return type
    auto return_type = ParseType();
    if (!return_type) {
        ReportError("Expected return type for method");
        return nullptr;
    }
    // Consume the opening bracket
    Consume(TokenKind::LEFT_BRACKET, "Expected '[' at start of method declaration");
    
    // Parse the receiver
    if (!Check(TokenKind::IDENTIFIER)) {
        ReportError("Expected identifier for method receiver");
        return nullptr;
    }
    
    std::string receiver = Peek().GetLexeme();
    Advance();
    
    // Parse the method selector and parameters
    if (!Check(TokenKind::IDENTIFIER)) {
        ReportError("Expected method name after receiver");
        return nullptr;
    }
    
    std::string selector = Peek().GetLexeme();
    Advance();
    
    std::vector<std::shared_ptr<ParamDecl>> parameters;
    
    // Check if we have parameters
    if (Match(TokenKind::COLON)) {
        // This is a method with at least one parameter
        do {
            auto param = ParseParameterDeclaration();
            if (param) {
                parameters.push_back(param);
                
                // If we have another parameter, it should be preceded by an identifier and colon
                if (Check(TokenKind::IDENTIFIER) && CheckNext(TokenKind::COLON)) {
                    std::string param_name = Peek().GetLexeme();
                    selector += "_" + param_name;
                    Advance(); // Consume identifier
                    Consume(TokenKind::COLON, "Expected ':' after parameter name");
                }
            }
        } while (Check(TokenKind::IDENTIFIER) && CheckNext(TokenKind::COLON));
    }
    
    // Consume the closing bracket
    Consume(TokenKind::RIGHT_BRACKET, "Expected ']' after method declaration");
    
    // Parse the method body
    std::shared_ptr<BlockStmt> body = nullptr;
    
    if (Match(TokenKind::SEMICOLON)) {
        // Method declaration (no body)
        return std::make_shared<MethodDecl>(selector, receiver, return_type, parameters, body);
    }
    
    // Method definition (with body)
    body = ParseBlockStatement();
    
    return std::make_shared<MethodDecl>(selector, receiver, return_type, parameters, body);
}

/**
 * ParseVariableDeclaration - Parse a variable declaration
 */
std::shared_ptr<VarDecl> Parser::ParseVariableDeclaration() {
    // Get the variable type
    auto type = ParseType();
    if (!type) {
        ReportError("Expected type for variable declaration");
        return nullptr;
    }
    
    // Get the variable name
    if (!Check(TokenKind::IDENTIFIER)) {
        ReportError("Expected variable name");
        return nullptr;
    }
    
    std::string name = Peek().GetLexeme();
    Advance();
    // Check for array declaration
    if (Match(TokenKind::LEFT_BRACKET)) {
        // Array type
        std::shared_ptr<Expr> size_expr = nullptr;
        
        if (!Check(TokenKind::RIGHT_BRACKET)) {
            size_expr = ParseExpression();
        }
        
        Consume(TokenKind::RIGHT_BRACKET, "Expected ']' after array size");
        
        // Create array type
        type = std::make_shared<ArrayType>(type, size_expr);
    }
    
    // Check for initializer
    std::shared_ptr<Expr> initializer = nullptr;
    
    if (Match(TokenKind::EQUAL)) {
        initializer = ParseExpression();
    }
    
    // Consume semicolon
    Consume(TokenKind::SEMICOLON, "Expected ';' after variable declaration");
    
    return std::make_shared<VarDecl>(name, type, initializer);
}

/**
 * ParseParameterDeclaration - Parse a parameter declaration
 */
std::shared_ptr<ParamDecl> Parser::ParseParameterDeclaration() {
    auto type = ParseType();
    if (!type) {
        ReportError("Expected parameter type");
        return nullptr;
    }
    
    if (!Check(TokenKind::IDENTIFIER)) {
        ReportError("Expected parameter name");
        return nullptr;
    }
    
    std::string name = Peek().GetLexeme();
    Advance();
    
    // Check for array notation
    if (Match(TokenKind::LEFT_BRACKET)) {
        Consume(TokenKind::RIGHT_BRACKET, "Expected ']' after array parameter");
        type = std::make_shared<ArrayType>(type, nullptr);
    }
    
    return std::make_shared<ParamDecl>(name, type);
}

/**
 * ParseStructDeclaration - Parse a struct declaration
 */
std::shared_ptr<StructDecl> Parser::ParseStructDeclaration() {
    // Parse struct name
    if (!Check(TokenKind::IDENTIFIER)) {
        ReportError("Expected struct name");
        return nullptr;
    }
    
    std::string name = Peek().GetLexeme();
    Advance();
    
    // Check for forward declaration
    if (Match(TokenKind::SEMICOLON)) {
        // Forward declaration
        return std::make_shared<StructDecl>(name, std::vector<std::shared_ptr<VarDecl>>());
    }
    
    // Parse struct body
    Consume(TokenKind::LEFT_BRACE, "Expected '{' after struct name");
    
    std::vector<std::shared_ptr<VarDecl>> fields;
    
    while (!Check(TokenKind::RIGHT_BRACE) && !IsAtEnd()) {
        auto type = ParseType();
        if (!type) {
            ReportError("Expected type for struct field");
            return nullptr;
        }
        
        if (!Check(TokenKind::IDENTIFIER)) {
            ReportError("Expected field name");
            return nullptr;
        }
        
        std::string field_name = Peek().GetLexeme();
        Advance();
        
        // Check for array field
        if (Match(TokenKind::LEFT_BRACKET)) {
            std::shared_ptr<Expr> size_expr = nullptr;
            
            if (!Check(TokenKind::RIGHT_BRACKET)) {
                size_expr = ParseExpression();
            }
            
            Consume(TokenKind::RIGHT_BRACKET, "Expected ']' after array size");
            
            // Create array type
            type = std::make_shared<ArrayType>(type, size_expr);
        }
        
        Consume(TokenKind::SEMICOLON, "Expected ';' after struct field declaration");
        
        fields.push_back(std::make_shared<VarDecl>(field_name, type, nullptr));
    }
    
    Consume(TokenKind::RIGHT_BRACE, "Expected '}' after struct body");
    Consume(TokenKind::SEMICOLON, "Expected ';' after struct declaration");
    
    return std::make_shared<StructDecl>(name, fields);
}

    /**
     * ParseEnumDeclaration - Parse an enum declaration
     */
    std::shared_ptr<EnumDecl> Parser::ParseEnumDeclaration() {
        // Parse enum name
        if (!Check(TokenKind::IDENTIFIER)) {
            ReportError("Expected enum name");
            return nullptr;
        }
        
        std::string name = Peek().GetLexeme();
        Advance();
        
        // Parse enum body
        Consume(TokenKind::LEFT_BRACE, "Expected '{' after enum name");
        
        // Use int64_t for enum values as required by EnumDecl constructor
        std::vector<std::pair<std::string, int64_t>> enumerators;
        int64_t next_value = 0;
        
        if (!Check(TokenKind::RIGHT_BRACE)) {
            do {
                if (!Check(TokenKind::IDENTIFIER)) {
                    ReportError("Expected enumerator name");
                    return nullptr;
                }
                
                std::string enum_name = Peek().GetLexeme();
                Advance();
                
                int64_t value = next_value++;
                
                if (Match(TokenKind::EQUAL)) {
                    // For simplicity, we only handle integer literals in enum values
                    // A more complete implementation would evaluate expressions
                    if (Check(TokenKind::INT_LITERAL)) {
                        try {
                            value = std::stoll(Peek().GetLexeme());
                            next_value = value + 1;
                        } catch (const std::exception& e) {
                            ReportError("Invalid integer literal in enum value");
                        }
                        Advance();
                    } else {
                        ReportError("Expected integer literal for enum value");
                        Advance(); // Skip the expression
                    }
                }
                
                enumerators.push_back(std::make_pair(enum_name, value));
            } while (Match(TokenKind::COMMA) && !Check(TokenKind::RIGHT_BRACE));
        }
        
        Consume(TokenKind::RIGHT_BRACE, "Expected '}' after enum body");
        Consume(TokenKind::SEMICOLON, "Expected ';' after enum declaration");
        
        // Assume a base type of int for all enums for now
        auto base_type = std::make_shared<IntType>();
        return std::make_shared<EnumDecl>(name, base_type, enumerators);
    }

/**
 * ParseType - Parse a type
 */
std::shared_ptr<Type> Parser::ParseType() {
    bool is_unsigned = false;
    
    // Check for unsigned keyword
    if (Match(TokenKind::KW_UNSIGNED)) {
        is_unsigned = true;
    }
    
    // Check for basic types
    if (Check(TokenKind::KW_VOID) || Check(TokenKind::KW_BOOL) ||
        Check(TokenKind::KW_CHAR) || Check(TokenKind::KW_SHORT) ||
        Check(TokenKind::KW_INT) || Check(TokenKind::KW_LONG) ||
        Check(TokenKind::KW_FLOAT) || Check(TokenKind::KW_DOUBLE)) {
        
        auto type = CreateType(Peek(), is_unsigned);
        Advance();
        
        // Check for pointer types
        while (Match(TokenKind::STAR)) {
            type = std::make_shared<PointerType>(type);
        }
        
        return type;
    }
    
    // Check for struct or enum type
    if (Match(TokenKind::KW_STRUCT)) {
        if (!Check(TokenKind::IDENTIFIER)) {
            ReportError("Expected struct name after 'struct'");
            return nullptr;
        }
        
        std::string name = Peek().GetLexeme();
        Advance();
        
        // Check if we've already created this struct type
        auto it = struct_types_.find(name);
        if (it != struct_types_.end()) {
            return it->second;
        }
        
        // Create a new struct type
        auto type = std::make_shared<StructType>(name);
        struct_types_[name] = type;
        
        // Check for pointer types
        while (Match(TokenKind::STAR)) {
            type = std::make_shared<PointerType>(type);
        }
        
        return type;
    }
    
    if (Match(TokenKind::KW_ENUM)) {
        if (!Check(TokenKind::IDENTIFIER)) {
            ReportError("Expected enum name after 'enum'");
            return nullptr;
        }
        
        std::string name = Peek().GetLexeme();
        Advance();
        
        // Check if we've already created this enum type
        auto it = enum_types_.find(name);
        if (it != enum_types_.end()) {
            return it->second;
        }
        
        // Create a new enum type
        auto base_type = std::make_shared<IntType>();
        auto type = std::make_shared<EnumType>(name, base_type);
        enum_types_[name] = type;
        
        // Check for pointer types
        while (Match(TokenKind::STAR)) {
            type = std::make_shared<PointerType>(type);
        }
        
        return type;
    }
    
    // If we reach here, it's not a valid type
    if (is_unsigned) {
        ReportError("Expected type after 'unsigned'");
    }
    
    return nullptr;
}

//===----------------------------------------------------------------------===//
// Statements
//===----------------------------------------------------------------------===//

/**
 * ParseStatement - Parse a statement
 */
std::shared_ptr<Stmt> Parser::ParseStatement() {
    if (Match(TokenKind::LEFT_BRACE)) {
        return ParseBlockStatement();
    }
    
    if (Match(TokenKind::KW_IF)) {
        return ParseIfStatement();
    }
    
    if (Match(TokenKind::KW_WHILE)) {
        return ParseWhileStatement();
    }
    
    if (Match(TokenKind::KW_FOR)) {
        return ParseForStatement();
    }
    
    if (Match(TokenKind::KW_RETURN)) {
        return ParseReturnStatement();
    }
    
    if (Match(TokenKind::KW_BREAK)) {
        return ParseBreakStatement();
    }
    
    if (Match(TokenKind::KW_CONTINUE)) {
        return ParseContinueStatement();
    }
    
    // Check for declarations
    if (Check(TokenKind::KW_VOID) || Check(TokenKind::KW_BOOL) ||
        Check(TokenKind::KW_CHAR) || Check(TokenKind::KW_SHORT) ||
        Check(TokenKind::KW_INT) || Check(TokenKind::KW_LONG) ||
        Check(TokenKind::KW_FLOAT) || Check(TokenKind::KW_
