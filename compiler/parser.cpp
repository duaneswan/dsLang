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

// Factory function to create array types
std::shared_ptr<ArrayType> CreateArrayType(std::shared_ptr<Type> element_type, std::shared_ptr<Expr> size_expr) {
    return std::make_shared<ArrayType>(element_type, size_expr);
}

std::shared_ptr<ArrayType> CreateArrayType(std::shared_ptr<Type> element_type, size_t size) {
    return std::make_shared<ArrayType>(element_type, size);
}

/**
 * CreateType - Create a type from the specified token
 */
std::shared_ptr<Type> Parser::CreateType(const Token& type_token, bool is_unsigned) {
    switch (type_token.GetKind()) {
        case TokenKind::KW_VOID:
            return std::make_shared<VoidType>();
        case TokenKind::KW_BOOL:
            return std::make_shared<BoolType>();
        case TokenKind::KW_CHAR:
            return std::make_shared<CharType>(is_unsigned);
        case TokenKind::KW_SHORT:
            return std::make_shared<ShortType>(is_unsigned);
        case TokenKind::KW_INT:
            return std::make_shared<IntType>(is_unsigned);
        case TokenKind::KW_LONG:
            return std::make_shared<LongType>(is_unsigned);
        case TokenKind::KW_FLOAT:
            return std::make_shared<FloatType>();
        case TokenKind::KW_DOUBLE:
            return std::make_shared<DoubleType>();
        default:
            ReportError("Unknown type token");
            return nullptr;
    }
}

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
// Type Parsing
//===----------------------------------------------------------------------===//

/**
 * ParseType - Parse a type
 */
std::shared_ptr<Type> Parser::ParseType() {
    bool is_unsigned = false;
    
    // Check for 'unsigned' modifier
    if (Match(TokenKind::KW_UNSIGNED)) {
        is_unsigned = true;
    }
    
    // Parse the base type
    if (Check(TokenKind::KW_VOID) || Check(TokenKind::KW_BOOL) ||
        Check(TokenKind::KW_CHAR) || Check(TokenKind::KW_SHORT) ||
        Check(TokenKind::KW_INT) || Check(TokenKind::KW_LONG) ||
        Check(TokenKind::KW_FLOAT) || Check(TokenKind::KW_DOUBLE)) {
        Token type_token = Peek();
        Advance();
        auto type = CreateType(type_token, is_unsigned);
        
        // Check for pointer type
        while (Match(TokenKind::STAR)) {
            type = std::make_shared<PointerType>(type);
        }
        
        return type;
    } else if (Match(TokenKind::KW_STRUCT)) {
        // Struct type
        if (!Check(TokenKind::IDENTIFIER)) {
            ReportError("Expected struct name");
            return nullptr;
        }
        
        std::string name = Peek().GetLexeme();
        Advance();
        
        auto type = std::make_shared<StructType>(name);
        
        // Check for pointer type
        while (Match(TokenKind::STAR)) {
            type = std::make_shared<PointerType>(type);
        }
        
        return type;
    } else if (Match(TokenKind::KW_ENUM)) {
        // Enum type
        if (!Check(TokenKind::IDENTIFIER)) {
            ReportError("Expected enum name");
            return nullptr;
        }
        
        std::string name = Peek().GetLexeme();
        Advance();
        
        auto type = std::make_shared<EnumType>(name);
        
        // Check for pointer type
        while (Match(TokenKind::STAR)) {
            type = std::make_shared<PointerType>(type);
        }
        
        return type;
    }
    
    ReportError("Expected type");
    return nullptr;
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
        auto decl = ParseDeclaration();
        if (decl) {
            declarations.push_back(decl);
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
    
    // For simplicity, assume all receivers are of type 'struct receiver'
    auto receiver_type = std::make_shared<StructType>(receiver);
    
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
        return std::make_shared<MethodDecl>(selector, return_type, receiver_type, parameters, body);
    }
    
    // Method definition (with body)
    body = ParseBlockStatement();
    
    return std::make_shared<MethodDecl>(selector, return_type, receiver_type, parameters, body);
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
        
        // Create a size_t literal for constant sizes if possible
        size_t array_size = 0;
        bool has_constant_size = false;
        
        if (size_expr && size_expr->GetType()->IsIntegral()) {
            auto literal = std::dynamic_pointer_cast<LiteralExpr>(size_expr);
            if (literal && literal->GetLiteralKind() == LiteralExpr::Kind::INT) {
                array_size = static_cast<size_t>(literal->GetIntValue());
                has_constant_size = true;
            }
        }
        
        // Create array type using the helper function
        if (has_constant_size) {
            type = CreateArrayType(type, array_size);
        } else if (size_expr) {
            type = CreateArrayType(type, size_expr);
        }
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
        
        // Create array type using the helper function with zero size for parameters
        type = CreateArrayType(type, (size_t)0);
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
            
            // Create array type using the helper function
            if (size_expr) {
                type = CreateArrayType(type, size_expr);
            } else {
                // For variable length arrays in struct fields, use size 0
                type = CreateArrayType(type, (size_t)0);
            }
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
                    value = std::stoll(Peek().GetLexeme());
                    Advance();
                    next_value = value + 1;
                } else {
                    ReportError("Expected integer literal for enum value");
                }
            }
            
            enumerators.push_back(std::make_pair(enum_name, value));
        } while (Match(TokenKind::COMMA) && !Check(TokenKind::RIGHT_BRACE));
    }
    
    Consume(TokenKind::RIGHT_BRACE, "Expected '}' after enum body");
    Consume(TokenKind::SEMICOLON, "Expected ';' after enum declaration");
    
    // Assume a base type of int for all enums for now
    auto base_type = std::make_shared<IntType>(false);
    return std::make_shared<EnumDecl>(name, base_type, enumerators);
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
        Check(TokenKind::KW_FLOAT) || Check(TokenKind::KW_DOUBLE) ||
        Check(TokenKind::KW_UNSIGNED) || Check(TokenKind::KW_STRUCT) ||
        Check(TokenKind::KW_ENUM)) {
        return ParseDeclarationStatement();
    }
    
    // Otherwise, this is an expression statement
    return ParseExpressionStatement();
}

/**
 * ParseBlockStatement - Parse a block statement
 */
std::shared_ptr<BlockStmt> Parser::ParseBlockStatement() {
    std::vector<std::shared_ptr<Stmt>> statements;
    
    // Parse statements until we reach the end of the block
    while (!Check(TokenKind::RIGHT_BRACE) && !IsAtEnd()) {
        auto stmt = ParseStatement();
        if (stmt) {
            statements.push_back(stmt);
        }
    }
    
    Consume(TokenKind::RIGHT_BRACE, "Expected '}' after block");
    
    return std::make_shared<BlockStmt>(statements);
}

/**
 * ParseExpressionStatement - Parse an expression statement
 */
std::shared_ptr<ExprStmt> Parser::ParseExpressionStatement() {
    auto expr = ParseExpression();
    
    Consume(TokenKind::SEMICOLON, "Expected ';' after expression");
    
    return std::make_shared<ExprStmt>(expr);
}

/**
 * ParseDeclarationStatement - Parse a declaration statement
 */
std::shared_ptr<DeclStmt> Parser::ParseDeclarationStatement() {
    // Parse a variable declaration
    auto decl = ParseVariableDeclaration();
    if (!decl) {
        ReportError("Expected variable declaration");
        return nullptr;
    }
    
    return std::make_shared<DeclStmt>(decl);
}

/**
 * ParseIfStatement - Parse an if statement
 */
std::shared_ptr<IfStmt> Parser::ParseIfStatement() {
    Consume(TokenKind::LEFT_PAREN, "Expected '(' after 'if'");
    auto condition = ParseExpression();
    Consume(TokenKind::RIGHT_PAREN, "Expected ')' after if condition");
    
    auto then_stmt = ParseStatement();
    std::shared_ptr<Stmt> else_stmt = nullptr;
    
    if (Match(TokenKind::KW_ELSE)) {
        else_stmt = ParseStatement();
    }
    
    return std::make_shared<IfStmt>(condition, then_stmt, else_stmt);
}

/**
 * ParseWhileStatement - Parse a while statement
 */
std::shared_ptr<WhileStmt> Parser::ParseWhileStatement() {
    Consume(TokenKind::LEFT_PAREN, "Expected '(' after 'while'");
    auto condition = ParseExpression();
    Consume(TokenKind::RIGHT_PAREN, "Expected ')' after while condition");
    
    auto body = ParseStatement();
    
    return std::make_shared<WhileStmt>(condition, body);
}

/**
 * ParseForStatement - Parse a for statement
 */
std::shared_ptr<ForStmt> Parser::ParseForStatement() {
    Consume(TokenKind::LEFT_PAREN, "Expected '(' after 'for'");
    
    // Parse initialization
    std::shared_ptr<Stmt> init = nullptr;
    if (!Check(TokenKind::SEMICOLON)) {
        if (Check(TokenKind::KW_VOID) || Check(TokenKind::KW_BOOL) ||
            Check(TokenKind::KW_CHAR) || Check(TokenKind::KW_SHORT) ||
            Check(TokenKind::KW_INT) || Check(TokenKind::KW_LONG) ||
            Check(TokenKind::KW_FLOAT) || Check(TokenKind::KW_DOUBLE) ||
            Check(TokenKind::KW_UNSIGNED) || Check(TokenKind::KW_STRUCT) ||
            Check(TokenKind::KW_ENUM)) {
            init = ParseDeclarationStatement();
        } else {
            init = ParseExpressionStatement();
        }
    } else {
        Consume(TokenKind::SEMICOLON, "Expected ';' after for initialization");
    }
    
    // Parse condition
    std::shared_ptr<Expr> condition = nullptr;
    if (!Check(TokenKind::SEMICOLON)) {
        condition = ParseExpression();
    }
    Consume(TokenKind::SEMICOLON, "Expected ';' after for condition");
    
    // Parse increment
    std::shared_ptr<Expr> increment = nullptr;
    if (!Check(TokenKind::RIGHT_PAREN)) {
        increment = ParseExpression();
    }
    Consume(TokenKind::RIGHT_PAREN, "Expected ')' after for clauses");
    
    // Parse body
    auto body = ParseStatement();
    
    return std::make_shared<ForStmt>(init, condition, increment, body);
}

/**
 * ParseReturnStatement - Parse a return statement
 */
std::shared_ptr<ReturnStmt> Parser::ParseReturnStatement() {
    std::shared_ptr<Expr> value = nullptr;
    
    // Check if the return statement has a value
    if (!Check(TokenKind::SEMICOLON)) {
        value = ParseExpression();
    }
    
    Consume(TokenKind::SEMICOLON, "Expected ';' after return value");
    
    return std::make_shared<ReturnStmt>(value);
}

/**
 * ParseBreakStatement - Parse a break statement
 */
std::shared_ptr<BreakStmt> Parser::ParseBreakStatement() {
    Consume(TokenKind::SEMICOLON, "Expected ';' after 'break'");
    
    return std::make_shared<BreakStmt>();
}

/**
 * ParseContinueStatement - Parse a continue statement
 */
std::shared_ptr<ContinueStmt> Parser::ParseContinueStatement() {
    Consume(TokenKind::SEMICOLON, "Expected ';' after 'continue'");
    
    return std::make_shared<ContinueStmt>();
}

//===----------------------------------------------------------------------===//
// Helper Methods
//===----------------------------------------------------------------------===//

/**
 * MakeBinaryExpr - Create a binary expression node
 */
std::shared_ptr<BinaryExpr> Parser::MakeBinaryExpr(BinaryExpr::Op op, 
                                                  std::shared_ptr<Expr> left, 
                                                  std::shared_ptr<Expr> right) {
    // Get the result type based on operand types and operator
    std::shared_ptr<Type> result_type;
    
    // Check if both operands have types
    if (left->GetType() && right->GetType()) {
        // Logical operators
        if (op == BinaryExpr::Op::LOGICAL_AND || op == BinaryExpr::Op::LOGICAL_OR) {
            result_type = std::make_shared<BoolType>();
        }
        // Equality operators
        else if (op == BinaryExpr::Op::EQUAL || op == BinaryExpr::Op::NOT_EQUAL) {
            result_type = std::make_shared<BoolType>();
