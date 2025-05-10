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
 * Parser constructor - Initialize the parser with a lexer
 */
Parser::Parser(Lexer& lexer, DiagnosticReporter& diag_reporter)
    : lexer_(lexer), diag_reporter_(diag_reporter) {
    // Initialize the current token
    current_token_ = lexer_.GetNextToken();
}

/**
 * Parse - Parse the source code and build an AST
 */
std::shared_ptr<CompilationUnit> Parser::Parse() {
    return ParseCompilationUnit();
}

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
 * Match - Check if the current token matches the expected kind
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
    Token prev = current_token_;
    current_token_ = lexer_.GetNextToken();
    return prev;
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
    return !IsAtEnd() && current_token_.GetKind() == kind;
}

/**
 * CheckNext - Check if the next token is of the expected kind
 */
bool Parser::CheckNext(TokenKind kind) const {
    return !IsAtEnd() && lexer_.PeekNextToken().GetKind() == kind;
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
    Synchronize();
}

/**
 * Synchronize - Synchronize after an error
 */
void Parser::Synchronize() {
    Advance();
    
    while (!IsAtEnd()) {
        // Skip until we find a token that could be the start of a new statement
        if (current_token_.GetKind() == TokenKind::SEMICOLON) {
            Advance();
            return;
        }
        
        switch (current_token_.GetKind()) {
            case TokenKind::KW_STRUCT:
            case TokenKind::KW_ENUM:
            case TokenKind::KW_IF:
            case TokenKind::KW_WHILE:
            case TokenKind::KW_FOR:
            case TokenKind::KW_RETURN:
            case TokenKind::KW_BREAK:
            case TokenKind::KW_CONTINUE:
                return;
            default:
                break;
        }
        
        Advance();
    }
}

/**
 * ParseCompilationUnit - Parse a compilation unit
 */
std::shared_ptr<CompilationUnit> Parser::ParseCompilationUnit() {
    std::vector<std::shared_ptr<Decl>> declarations;
    
    while (!IsAtEnd()) {
        try {
            auto decl = ParseDeclaration();
            if (decl) {
                declarations.push_back(decl);
            }
        } catch (const std::exception& e) {
            ReportError(e.what());
            Synchronize();
        }
    }
    
    return std::make_shared<CompilationUnit>(declarations);
}

/**
 * ParseDeclaration - Parse a declaration
 */
std::shared_ptr<Decl> Parser::ParseDeclaration() {
    if (Match(TokenKind::KW_STRUCT)) {
        return ParseStructDeclaration();
    }
    
    if (Match(TokenKind::KW_ENUM)) {
        return ParseEnumDeclaration();
    }
    
    // Look for function, method, or variable declarations
    // Parse type first
    std::shared_ptr<Type> type = ParseType();
    
    // Check for identifier
    if (!Check(TokenKind::IDENTIFIER)) {
        ReportError("Expected identifier after type");
        return nullptr;
    }
    
    std::string name = current_token_.GetLexeme();
    Advance();
    
    // Function or method declaration
    if (Check(TokenKind::LEFT_PAREN)) {
        return ParseFunctionDeclaration();
    }
    
    // Variable declaration
    return ParseVariableDeclaration();
}

/**
 * ParseType - Parse a type
 */
std::shared_ptr<Type> Parser::ParseType() {
    bool is_unsigned = false;
    
    // Check for unsigned qualifier
    if (Match(TokenKind::KW_UNSIGNED)) {
        is_unsigned = true;
    }
    
    // Check for basic types
    if (Match(TokenKind::KW_VOID) || Match(TokenKind::KW_BOOL) ||
        Match(TokenKind::KW_CHAR) || Match(TokenKind::KW_SHORT) ||
        Match(TokenKind::KW_INT) || Match(TokenKind::KW_LONG) ||
        Match(TokenKind::KW_FLOAT) || Match(TokenKind::KW_DOUBLE)) {
        
        // Create the type using the token that was matched (which is now the previous token)
        auto type = CreateType(Advance(), is_unsigned);
        
        // Check for pointer type
        while (Match(TokenKind::STAR)) {
            type = std::make_shared<PointerType>(type);
        }
        
        // Check for array type
        if (Match(TokenKind::LEFT_BRACKET)) {
            if (Match(TokenKind::RIGHT_BRACKET)) {
                // Unsized array, treat as pointer for now
                type = std::make_shared<PointerType>(type);
            } else {
                // Sized array
                auto size_expr = ParseExpression();
                Consume(TokenKind::RIGHT_BRACKET, "Expected ']' after array size");
                type = CreateArrayType(type, size_expr);
            }
        }
        
        return type;
    }
    
    // Check for struct type
    if (Match(TokenKind::KW_STRUCT)) {
        if (!Check(TokenKind::IDENTIFIER)) {
            ReportError("Expected struct name");
            return nullptr;
        }
        
        std::string name = current_token_.GetLexeme();
        Advance();
        
        // Try to find existing struct type
        auto struct_it = struct_types_.find(name);
        if (struct_it != struct_types_.end()) {
            std::shared_ptr<Type> type = struct_it->second;
            
            // Check for pointer type
            while (Match(TokenKind::STAR)) {
                type = std::make_shared<PointerType>(type);
            }
            
            return type;
        }
        
        // Create a new struct type
        auto struct_type = std::make_shared<StructType>(name);
        struct_types_.insert(std::make_pair(name, struct_type));
        std::shared_ptr<Type> type = struct_type;
        
        // Check for pointer type
        while (Match(TokenKind::STAR)) {
            type = std::make_shared<PointerType>(type);
        }
        
        return type;
    }
    
    // Check for enum type
    if (Match(TokenKind::KW_ENUM)) {
        if (!Check(TokenKind::IDENTIFIER)) {
            ReportError("Expected enum name");
            return nullptr;
        }
        
        std::string name = current_token_.GetLexeme();
        Advance();
        
        // Try to find existing enum type
        auto enum_it = enum_types_.find(name);
        if (enum_it != enum_types_.end()) {
            std::shared_ptr<Type> type = enum_it->second;
            
            // Check for pointer type
            while (Match(TokenKind::STAR)) {
                type = std::make_shared<PointerType>(type);
            }
            
            return type;
        }
        
        // Create a new enum type - use IntType as base
        auto base_type = std::make_shared<IntType>();
        auto enum_type = std::make_shared<EnumType>(name, base_type);
        enum_types_.insert(std::make_pair(name, enum_type));
        std::shared_ptr<Type> type = enum_type;
        
        // Check for pointer type
        while (Match(TokenKind::STAR)) {
            type = std::make_shared<PointerType>(type);
        }
        
        return type;
    }
    
    ReportError("Expected type");
    return nullptr;
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
            if (is_unsigned) {
                return std::make_shared<CharType>(PrimitiveType::SignKind::UNSIGNED);
            } else {
                return std::make_shared<CharType>();
            }
        case TokenKind::KW_SHORT:
            if (is_unsigned) {
                return std::make_shared<ShortType>(PrimitiveType::SignKind::UNSIGNED);
            } else {
                return std::make_shared<ShortType>();
            }
        case TokenKind::KW_INT:
            if (is_unsigned) {
                return std::make_shared<IntType>(PrimitiveType::SignKind::UNSIGNED);
            } else {
                return std::make_shared<IntType>();
            }
        case TokenKind::KW_LONG:
            if (is_unsigned) {
                return std::make_shared<LongType>(PrimitiveType::SignKind::UNSIGNED);
            } else {
                return std::make_shared<LongType>();
            }
        case TokenKind::KW_FLOAT:
            return std::make_shared<FloatType>();
        case TokenKind::KW_DOUBLE:
            return std::make_shared<DoubleType>();
        default:
            // Unreachable
            return std::make_shared<IntType>();
    }
}

/**
 * ParseStructDeclaration - Parse a struct declaration
 */
std::shared_ptr<StructDecl> Parser::ParseStructDeclaration() {
    // Struct name
    if (!Check(TokenKind::IDENTIFIER)) {
        ReportError("Expected struct name");
        return nullptr;
    }
    
    std::string name = current_token_.GetLexeme();
    Advance();
    
    // Get or create struct type
    std::shared_ptr<StructType> type;
    auto struct_it = struct_types_.find(name);
    if (struct_it != struct_types_.end()) {
        type = struct_it->second;
    } else {
        type = std::make_shared<StructType>(name);
        struct_types_.insert(std::make_pair(name, type));
    }
    
    std::vector<std::shared_ptr<VarDecl>> fields;
    
    // Parse struct body
    Consume(TokenKind::LEFT_BRACE, "Expected '{' after struct name");
    
    while (!Check(TokenKind::RIGHT_BRACE) && !IsAtEnd()) {
        // Parse field
        auto field_type = ParseType();
        
        if (!Check(TokenKind::IDENTIFIER)) {
            ReportError("Expected field name");
            continue;
        }
        
        std::string field_name = current_token_.GetLexeme();
        Advance();
        
        std::shared_ptr<Expr> initializer = nullptr;
        
        // Check for initializer
        if (Match(TokenKind::EQUAL)) {
            initializer = ParseExpression();
        }
        
        Consume(TokenKind::SEMICOLON, "Expected ';' after field declaration");
        
        fields.push_back(std::make_shared<VarDecl>(field_name, field_type, initializer));
    }
    
    Consume(TokenKind::RIGHT_BRACE, "Expected '}' after struct body");
    
    return std::make_shared<StructDecl>(name, fields);
}

/**
 * ParseEnumDeclaration - Parse an enum declaration
 */
std::shared_ptr<EnumDecl> Parser::ParseEnumDeclaration() {
    // Enum name
    if (!Check(TokenKind::IDENTIFIER)) {
        ReportError("Expected enum name");
        return nullptr;
    }
    
    std::string name = current_token_.GetLexeme();
    Advance();
    
    // Get or create enum type
    std::shared_ptr<EnumType> type;
    auto enum_it = enum_types_.find(name);
    if (enum_it != enum_types_.end()) {
        type = enum_it->second;
    } else {
        // Create a new enum type with IntType as base
        auto base_type = std::make_shared<IntType>();
        type = std::make_shared<EnumType>(name, base_type);
        enum_types_.insert(std::make_pair(name, type));
    }
    
    std::vector<std::pair<std::string, std::shared_ptr<Expr>>> values;
    
    // Parse enum body
    Consume(TokenKind::LEFT_BRACE, "Expected '{' after enum name");
    
    while (!Check(TokenKind::RIGHT_BRACE) && !IsAtEnd()) {
        // Parse enum value
        if (!Check(TokenKind::IDENTIFIER)) {
            ReportError("Expected enum value name");
            Synchronize();
            continue;
        }
        
        std::string value_name = current_token_.GetLexeme();
        Advance();
        
        std::shared_ptr<Expr> value = nullptr;
        
        // Check for explicit value
        if (Match(TokenKind::EQUAL)) {
            value = ParseExpression();
        } else {
            // If no explicit value, use previous value + 1 or 0 if first
            if (values.empty()) {
                // First value defaults to 0
                auto int_type = std::make_shared<IntType>();
                value = std::make_shared<LiteralExpr>(0LL, int_type);
            } else {
                // Create an expression that adds 1 to the previous value
                auto prev_value = values.back().second;
                auto int_type = std::make_shared<IntType>();
                auto one = std::make_shared<LiteralExpr>(1LL, int_type);
                value = std::make_shared<BinaryExpr>(BinaryExpr::Op::ADD, prev_value, one, int_type);
            }
        }
        
        values.push_back(std::make_pair(value_name, value));
        
        // Comma after enum value is optional for the last value
        if (!Check(TokenKind::RIGHT_BRACE)) {
            Consume(TokenKind::COMMA, "Expected ',' after enum value");
        }
    }
    
    Consume(TokenKind::RIGHT_BRACE, "Expected '}' after enum body");
    
    // Extract values for the enum declaration
    std::vector<std::pair<std::string, int64_t>> value_pairs;
    for (const auto& pair : values) {
        // We should properly evaluate the expressions here, but for simplicity,
        // we'll just assume they're all literals
        auto literal = std::dynamic_pointer_cast<LiteralExpr>(pair.second);
        if (literal && literal->GetLiteralKind() == LiteralExpr::Kind::INT) {
            value_pairs.push_back(std::make_pair(pair.first, literal->GetIntValue()));
        } else {
            // Default to 0 if not a literal
            value_pairs.push_back(std::make_pair(pair.first, 0));
        }
    }
    
    return std::make_shared<EnumDecl>(name, type->GetBaseType(), value_pairs);
}

/**
 * ParseFunctionDeclaration - Parse a function declaration
 */
std::shared_ptr<FuncDecl> Parser::ParseFunctionDeclaration() {
    // Function name
    if (!Check(TokenKind::IDENTIFIER)) {
        ReportError("Expected function name");
        return nullptr;
    }
    
    std::string name = current_token_.GetLexeme();
    Advance();
    
    // Function return type
    auto return_type = ParseType();
    
    // Function parameters
    Consume(TokenKind::LEFT_PAREN, "Expected '(' after function name");
    
    std::vector<std::shared_ptr<ParamDecl>> parameters;
    
    if (!Check(TokenKind::RIGHT_PAREN)) {
        do {
            auto param = ParseParameterDeclaration();
            if (param) {
                parameters.push_back(param);
            }
        } while (Match(TokenKind::COMMA));
    }
    
    Consume(TokenKind::RIGHT_PAREN, "Expected ')' after function parameters");
    
    // Function body
    std::shared_ptr<BlockStmt> body = nullptr;
    
    if (Match(TokenKind::SEMICOLON)) {
        // Function declaration without body
    } else {
        body = ParseBlockStatement();
    }
    
    return std::make_shared<FuncDecl>(name, return_type, parameters, body);
}

/**
 * ParseMethodDeclaration - Parse a method declaration (Objective-C style)
 */
std::shared_ptr<MethodDecl> Parser::ParseMethodDeclaration() {
    // Method return type
    auto return_type = ParseType();
    
    // Method receiver type
    Consume(TokenKind::LEFT_PAREN, "Expected '(' after 'method'");
    auto receiver_type = ParseType();
    Consume(TokenKind::RIGHT_PAREN, "Expected ')' after method receiver type");
    
    // Method selector
    if (!Check(TokenKind::IDENTIFIER)) {
        ReportError("Expected method selector");
        return nullptr;
    }
    
    std::string selector = current_token_.GetLexeme();
    Advance();
    
    // Method parameters
    std::vector<std::shared_ptr<ParamDecl>> parameters;
    std::vector<std::string> selector_parts;
    
    if (Match(TokenKind::COLON)) {
        // This is a method with at least one parameter
        do {
            // Parameter
            auto param = ParseParameterDeclaration();
            if (param) {
                parameters.push_back(param);
            }
            
            // Next selector part
            if (Check(TokenKind::IDENTIFIER) && CheckNext(TokenKind::COLON)) {
                selector_parts.push_back(current_token_.GetLexeme());
                Advance();
                Consume(TokenKind::COLON, "Expected ':' after selector part");
            }
        } while (Check(TokenKind::IDENTIFIER) && CheckNext(TokenKind::COLON));
    }
    
    // Build full selector
    std::string full_selector = selector;
    for (const auto& part : selector_parts) {
        full_selector += "_" + part;
    }
    
    // Method body
    std::shared_ptr<BlockStmt> body = nullptr;
    
    if (Match(TokenKind::SEMICOLON)) {
        // Method declaration without body
    } else {
        body = ParseBlockStatement();
    }
    
    return std::make_shared<MethodDecl>(full_selector, return_type, receiver_type, parameters, body);
}

/**
 * ParseParameterDeclaration - Parse a parameter declaration
 */
std::shared_ptr<ParamDecl> Parser::ParseParameterDeclaration() {
    // Parameter type
    auto type = ParseType();
    
    // Parameter name
    if (!Check(TokenKind::IDENTIFIER)) {
        ReportError("Expected parameter name");
        return nullptr;
    }
    
    std::string name = current_token_.GetLexeme();
    Advance();
    
    return std::make_shared<ParamDecl>(name, type);
}

/**
 * ParseVariableDeclaration - Parse a variable declaration
 */
std::shared_ptr<VarDecl> Parser::ParseVariableDeclaration() {
    // Variable type
    auto type = ParseType();
    
    // Variable name
    if (!Check(TokenKind::IDENTIFIER)) {
        ReportError("Expected variable name");
        return nullptr;
    }
    
    std::string name = current_token_.GetLexeme();
    Advance();
    
    std::shared_ptr<Expr> initializer = nullptr;
    
    // Check for initializer
    if (Match(TokenKind::EQUAL)) {
        initializer = ParseExpression();
    }
    
    Consume(TokenKind::SEMICOLON, "Expected ';' after variable declaration");
    
    return std::make_shared<VarDecl>(name, type, initializer);
}

/**
 * ParseStatement - Parse a statement
 */
std::shared_ptr<Stmt> Parser::ParseStatement() {
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
    
    if (Match(TokenKind::LEFT_BRACE)) {
        return ParseBlockStatement();
    }
    
    // Check for declaration statement
    if (Check(TokenKind::KW_VOID) || Check(TokenKind::KW_BOOL) ||
        Check(TokenKind::KW_CHAR) || Check(TokenKind::KW_SHORT) ||
        Check(TokenKind::KW_INT) || Check(TokenKind::KW_LONG) ||
        Check(TokenKind::KW_FLOAT) || Check(TokenKind::KW_DOUBLE) ||
        Check(TokenKind::KW_UNSIGNED) || Check(TokenKind::KW_STRUCT) ||
        Check(TokenKind::KW_ENUM)) {
        
        return ParseDeclarationStatement();
    }
    
    return ParseExpressionStatement();
}

/**
 * ParseBlockStatement - Parse a block statement
 */
std::shared_ptr<BlockStmt> Parser::ParseBlockStatement() {
    std::vector<std::shared_ptr<Stmt>> statements;
    
    while (!Check(TokenKind::RIGHT_BRACE) && !IsAtEnd()) {
        statements.push_back(ParseStatement());
    }
    
    Consume(TokenKind::RIGHT_BRACE, "Expected '}' after block");
    
    return std::make_shared<BlockStmt>(statements);
}

/**
 * ParseIfStatement - Parse an if statement
 */
std::shared_ptr<IfStmt> Parser::ParseIfStatement() {
    Consume(TokenKind::LEFT_PAREN, "Expected '(' after 'if'");
    auto condition = ParseExpression();
    Consume(TokenKind::RIGHT_PAREN, "Expected ')' after if condition");
    
    auto then_branch = ParseStatement();
    std::shared_ptr<Stmt> else_branch = nullptr;
    
    if (Match(TokenKind::KW_ELSE)) {
        else_branch = ParseStatement();
    }
    
    return std::make_shared<IfStmt>(condition, then_branch, else_branch);
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
    
    std::shared_ptr<Stmt> initializer = nullptr;
    std::shared_ptr<Expr> condition = nullptr;
    std::shared_ptr<Expr> increment = nullptr;
    
    // Initializer
    if (Match(TokenKind::SEMICOLON)) {
        // No initializer
    } else if (Check(TokenKind::KW_VOID) || Check(TokenKind::KW_BOOL) ||
               Check(TokenKind::KW_CHAR) || Check(TokenKind::KW_SHORT) ||
               Check(TokenKind::KW_INT) || Check(TokenKind::KW_LONG) ||
               Check(TokenKind::KW_FLOAT) || Check(TokenKind::KW_DOUBLE) ||
               Check(TokenKind::KW_UNSIGNED) || Check(TokenKind::KW_STRUCT) ||
               Check(TokenKind::KW_ENUM)) {
        initializer = ParseDeclarationStatement();
    } else {
        initializer = ParseExpressionStatement();
    }
    
    // Condition
    if (!Check(TokenKind::SEMICOLON)) {
        condition = ParseExpression();
    }
    
    Consume(TokenKind::SEMICOLON, "Expected ';' after for condition");
    
    // Increment
    if (!Check(TokenKind::RIGHT_PAREN)) {
        increment = ParseExpression();
    }
    
    Consume(TokenKind::RIGHT_PAREN, "Expected ')' after for clauses");
    
    auto body = ParseStatement();
    
    return std::make_shared<ForStmt>(initializer, condition, increment, body);
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
    auto decl = std::dynamic_pointer_cast<VarDecl>(ParseDeclaration());
    
    if (!decl) {
        ReportError("Expected variable declaration");
        return nullptr;
    }
    
    return std::make_shared<DeclStmt>(decl);
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
// Expressions
//===----------------------------------------------------------------------===//

/**
 * ParseExpression - Parse an expression
 * 
 * This is the top-level expression parsing function.
 */
std::shared_ptr<Expr> Parser::ParseExpression() {
    return ParseAssignment();
}

/**
 * ParseAssignment - Parse an assignment expression
 */
std::shared_ptr<Expr> Parser::ParseAssignment() {
    auto expr = ParseLogicalOr();
    
    if (Match(TokenKind::EQUAL)) {
        auto value = ParseAssignment();
        
        // Check that the left-hand side is a valid assignment target
        if (std::dynamic_pointer_cast<VarExpr>(expr) ||
            std::dynamic_pointer_cast<SubscriptExpr>(expr)) {
            return std::make_shared<AssignExpr>(expr, value);
        }
        
        ReportError("Invalid assignment target");
    }
    
    return expr;
}

/**
 * ParseLogicalOr - Parse a logical OR expression
 */
std::shared_ptr
