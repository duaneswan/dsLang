/**
 * parser.h - Parser Interface for dsLang
 * 
 * This file defines the Parser class which is responsible for parsing
 * source tokens into an Abstract Syntax Tree (AST).
 */

#ifndef DSLANG_PARSER_H
#define DSLANG_PARSER_H

#include "lexer.h"
#include "ast.h"
#include "type.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace dsLang {

class DiagnosticReporter;

/**
 * Parser - Parser for dsLang
 * 
 * The parser converts a sequence of tokens into an Abstract Syntax Tree (AST).
 * It uses a recursive descent parsing approach.
 */
class Parser {
public:
    /**
     * Constructor - Initialize the parser with a lexer
     * 
     * @param lexer The lexer to get tokens from
     * @param diag_reporter The diagnostic reporter for error handling
     */
    Parser(Lexer& lexer, DiagnosticReporter& diag_reporter);
    
    /**
     * Parse - Parse the source code and build an AST
     * 
     * @return The root node of the AST
     */
    std::shared_ptr<CompilationUnit> Parse();
    
    /**
     * HasErrors - Check if any errors were encountered during parsing
     * 
     * @return True if errors were encountered, false otherwise
     */
    bool HasErrors() const { return has_errors_; }
    
private:
    /**
     * Consume - Consume the current token if it matches the expected kind
     * 
     * @param kind The expected token kind
     * @param error_msg The error message to report if the token doesn't match
     * @return True if the token was consumed, false otherwise
     */
    bool Consume(TokenKind kind, const std::string& error_msg);
    
    /**
     * Match - Check if the current token matches the expected kind
     * 
     * @param kind The expected token kind
     * @return True if the token matches, false otherwise
     */
    bool Match(TokenKind kind);
    
    /**
     * Advance - Advance to the next token
     * 
     * @return The consumed token
     */
    Token Advance();
    
    /**
     * Peek - Peek at the current token
     * 
     * @return The current token
     */
    Token Peek() const;
    
    /**
     * PeekNext - Peek at the next token
     * 
     * @return The next token
     */
    Token PeekNext() const;
    
    /**
     * Check - Check if the current token is of the expected kind
     * 
     * @param kind The expected token kind
     * @return True if the token is of the expected kind, false otherwise
     */
    bool Check(TokenKind kind) const;
    
    /**
     * CheckNext - Check if the next token is of the expected kind
     * 
     * @param kind The expected token kind
     * @return True if the token is of the expected kind, false otherwise
     */
    bool CheckNext(TokenKind kind) const;
    
    /**
     * IsAtEnd - Check if we've reached the end of the token stream
     * 
     * @return True if we're at the end, false otherwise
     */
    bool IsAtEnd() const;
    
    /**
     * ReportError - Report an error at the current token
     * 
     * @param message The error message
     */
    void ReportError(const std::string& message);
    
    /**
     * Synchronize - Synchronize after an error
     * 
     * Skip tokens until we find a token that could be the start of a new statement.
     */
    void Synchronize();
    
    //===----------------------------------------------------------------------===//
    // Declarations
    //===----------------------------------------------------------------------===//
    
    /**
     * ParseCompilationUnit - Parse a compilation unit
     * 
     * @return The compilation unit node
     */
    std::shared_ptr<CompilationUnit> ParseCompilationUnit();
    
    /**
     * ParseDeclaration - Parse a declaration
     * 
     * @return The declaration node
     */
    std::shared_ptr<Decl> ParseDeclaration();
    
    /**
     * ParseFunctionDeclaration - Parse a function declaration
     * 
     * @return The function declaration node
     */
    std::shared_ptr<FuncDecl> ParseFunctionDeclaration();
    
    /**
     * ParseMethodDeclaration - Parse a method declaration (Objective-C style)
     * 
     * @return The method declaration node
     */
    std::shared_ptr<MethodDecl> ParseMethodDeclaration();
    
    /**
     * ParseVariableDeclaration - Parse a variable declaration
     * 
     * @return The variable declaration node
     */
    std::shared_ptr<VarDecl> ParseVariableDeclaration();
    
    /**
     * ParseParameterDeclaration - Parse a parameter declaration
     * 
     * @return The parameter declaration node
     */
    std::shared_ptr<ParamDecl> ParseParameterDeclaration();
    
    /**
     * ParseStructDeclaration - Parse a struct declaration
     * 
     * @return The struct declaration node
     */
    std::shared_ptr<StructDecl> ParseStructDeclaration();
    
    /**
     * ParseEnumDeclaration - Parse an enum declaration
     * 
     * @return The enum declaration node
     */
    std::shared_ptr<EnumDecl> ParseEnumDeclaration();
    
    /**
     * ParseType - Parse a type
     * 
     * @return The type
     */
    std::shared_ptr<Type> ParseType();
    
    //===----------------------------------------------------------------------===//
    // Statements
    //===----------------------------------------------------------------------===//
    
    /**
     * ParseStatement - Parse a statement
     * 
     * @return The statement node
     */
    std::shared_ptr<Stmt> ParseStatement();
    
    /**
     * ParseBlockStatement - Parse a block statement
     * 
     * @return The block statement node
     */
    std::shared_ptr<BlockStmt> ParseBlockStatement();
    
    /**
     * ParseExpressionStatement - Parse an expression statement
     * 
     * @return The expression statement node
     */
    std::shared_ptr<ExprStmt> ParseExpressionStatement();
    
    /**
     * ParseIfStatement - Parse an if statement
     * 
     * @return The if statement node
     */
    std::shared_ptr<IfStmt> ParseIfStatement();
    
    /**
     * ParseWhileStatement - Parse a while statement
     * 
     * @return The while statement node
     */
    std::shared_ptr<WhileStmt> ParseWhileStatement();
    
    /**
     * ParseForStatement - Parse a for statement
     * 
     * @return The for statement node
     */
    std::shared_ptr<ForStmt> ParseForStatement();
    
    /**
     * ParseReturnStatement - Parse a return statement
     * 
     * @return The return statement node
     */
    std::shared_ptr<ReturnStmt> ParseReturnStatement();
    
    /**
     * ParseBreakStatement - Parse a break statement
     * 
     * @return The break statement node
     */
    std::shared_ptr<BreakStmt> ParseBreakStatement();
    
    /**
     * ParseContinueStatement - Parse a continue statement
     * 
     * @return The continue statement node
     */
    std::shared_ptr<ContinueStmt> ParseContinueStatement();
    
    /**
     * ParseDeclarationStatement - Parse a declaration statement
     * 
     * @return The declaration statement node
     */
    std::shared_ptr<DeclStmt> ParseDeclarationStatement();
    
    //===----------------------------------------------------------------------===//
    // Expressions
    //===----------------------------------------------------------------------===//
    
    /**
     * ParseExpression - Parse an expression
     * 
     * @return The expression node
     */
    std::shared_ptr<Expr> ParseExpression();
    
    /**
     * ParseAssignment - Parse an assignment expression
     * 
     * @return The assignment expression node
     */
    std::shared_ptr<Expr> ParseAssignment();
    
    /**
     * ParseLogicalOr - Parse a logical OR expression
     * 
     * @return The logical OR expression node
     */
    std::shared_ptr<Expr> ParseLogicalOr();
    
    /**
     * ParseLogicalAnd - Parse a logical AND expression
     * 
     * @return The logical AND expression node
     */
    std::shared_ptr<Expr> ParseLogicalAnd();
    
    /**
     * ParseEquality - Parse an equality expression
     * 
     * @return The equality expression node
     */
    std::shared_ptr<Expr> ParseEquality();
    
    /**
     * ParseComparison - Parse a comparison expression
     * 
     * @return The comparison expression node
     */
    std::shared_ptr<Expr> ParseComparison();
    
    /**
     * ParseBitwiseOr - Parse a bitwise OR expression
     * 
     * @return The bitwise OR expression node
     */
    std::shared_ptr<Expr> ParseBitwiseOr();
    
    /**
     * ParseBitwiseXor - Parse a bitwise XOR expression
     * 
     * @return The bitwise XOR expression node
     */
    std::shared_ptr<Expr> ParseBitwiseXor();
    
    /**
     * ParseBitwiseAnd - Parse a bitwise AND expression
     * 
     * @return The bitwise AND expression node
     */
    std::shared_ptr<Expr> ParseBitwiseAnd();
    
    /**
     * ParseShift - Parse a shift expression
     * 
     * @return The shift expression node
     */
    std::shared_ptr<Expr> ParseShift();
    
    /**
     * ParseAdditive - Parse an additive expression
     * 
     * @return The additive expression node
     */
    std::shared_ptr<Expr> ParseAdditive();
    
    /**
     * ParseMultiplicative - Parse a multiplicative expression
     * 
     * @return The multiplicative expression node
     */
    std::shared_ptr<Expr> ParseMultiplicative();
    
    /**
     * ParseUnary - Parse a unary expression
     * 
     * @return The unary expression node
     */
    std::shared_ptr<Expr> ParseUnary();
    
    /**
     * ParsePostfix - Parse a postfix expression
     * 
     * @return The postfix expression node
     */
    std::shared_ptr<Expr> ParsePostfix();
    
    /**
     * ParsePrimary - Parse a primary expression
     * 
     * @return The primary expression node
     */
    std::shared_ptr<Expr> ParsePrimary();
    
    /**
     * ParseMessageExpression - Parse a message expression (Objective-C style)
     * 
     * @return The message expression node
     */
    std::shared_ptr<Expr> ParseMessageExpression();
    
    /**
     * ParseFunctionCall - Parse a function call
     * 
     * @param callee The function being called
     * @return The function call expression node
     */
    std::shared_ptr<Expr> ParseFunctionCall(std::shared_ptr<Expr> callee);
    
    /**
     * ParseSubscript - Parse an array subscript
     * 
     * @param array The array expression
     * @return The subscript expression node
     */
    std::shared_ptr<Expr> ParseSubscript(std::shared_ptr<Expr> array);
    
    /**
     * ParseCastExpression - Parse a cast expression
     * 
     * @return The cast expression node
     */
    std::shared_ptr<Expr> ParseCastExpression();
    
    //===----------------------------------------------------------------------===//
    // Helper Methods
    //===----------------------------------------------------------------------===//
    
    /**
     * MakeBinaryExpr - Create a binary expression node
     * 
     * @param op The operator
     * @param left The left operand
     * @param right The right operand
     * @return The binary expression node
     */
    std::shared_ptr<BinaryExpr> MakeBinaryExpr(BinaryExpr::Op op, 
                                               std::shared_ptr<Expr> left, 
                                               std::shared_ptr<Expr> right);
    
    /**
     * MakeUnaryExpr - Create a unary expression node
     * 
     * @param op The operator
     * @param operand The operand
     * @return The unary expression node
     */
    std::shared_ptr<UnaryExpr> MakeUnaryExpr(UnaryExpr::Op op, 
                                             std::shared_ptr<Expr> operand);
    
    /**
     * CreateType - Create a type from the specified tokens
     * 
     * @param type_token The type token
     * @param is_unsigned Whether the type is unsigned
     * @return The type
     */
    std::shared_ptr<Type> CreateType(const Token& type_token, bool is_unsigned);
    
private:
    Lexer& lexer_;                                   // The lexer to get tokens from
    DiagnosticReporter& diag_reporter_;              // The diagnostic reporter
    Token current_token_;                            // The current token
    bool has_errors_ = false;                        // Whether any errors were encountered
    
    // Type cache to avoid creating duplicate types
    std::unordered_map<std::string, std::shared_ptr<StructType>> struct_types_;
    std::unordered_map<std::string, std::shared_ptr<EnumType>> enum_types_;
};

} // namespace dsLang

#endif // DSLANG_PARSER_H
