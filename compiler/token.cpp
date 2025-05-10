/**
 * token.cpp - Token Implementation for dsLang
 * 
 * This file implements token-related functionality for the lexical analyzer.
 */

#include "token.h"
#include <unordered_map>

namespace dsLang {

/**
 * GetTokenName - Get a string representation of the token kind
 */
std::string Token::GetTokenName() const {
    static const std::unordered_map<TokenKind, std::string> token_names = {
        // End of file
        {TokenKind::END_OF_FILE, "EOF"},
        
        // Literals
        {TokenKind::IDENTIFIER, "IDENTIFIER"},
        {TokenKind::INT_LITERAL, "INT_LITERAL"},
        {TokenKind::FLOAT_LITERAL, "FLOAT_LITERAL"},
        {TokenKind::CHAR_LITERAL, "CHAR_LITERAL"},
        {TokenKind::STRING_LITERAL, "STRING_LITERAL"},
        
        // Keywords
        {TokenKind::KW_IF, "if"},
        {TokenKind::KW_ELSE, "else"},
        {TokenKind::KW_WHILE, "while"},
        {TokenKind::KW_FOR, "for"},
        {TokenKind::KW_DO, "do"},
        {TokenKind::KW_BREAK, "break"},
        {TokenKind::KW_CONTINUE, "continue"},
        {TokenKind::KW_RETURN, "return"},
        {TokenKind::KW_VOID, "void"},
        {TokenKind::KW_BOOL, "bool"},
        {TokenKind::KW_CHAR, "char"},
        {TokenKind::KW_SHORT, "short"},
        {TokenKind::KW_INT, "int"},
        {TokenKind::KW_LONG, "long"},
        {TokenKind::KW_FLOAT, "float"},
        {TokenKind::KW_DOUBLE, "double"},
        {TokenKind::KW_UNSIGNED, "unsigned"},
        {TokenKind::KW_STRUCT, "struct"},
        {TokenKind::KW_ENUM, "enum"},
        {TokenKind::KW_CONST, "const"},
        {TokenKind::KW_TRUE, "true"},
        {TokenKind::KW_FALSE, "false"},
        {TokenKind::KW_NULL, "null"},
        
        // Operators
        {TokenKind::PLUS, "+"},
        {TokenKind::MINUS, "-"},
        {TokenKind::STAR, "*"},
        {TokenKind::SLASH, "/"},
        {TokenKind::PERCENT, "%"},
        {TokenKind::AMP, "&"},
        {TokenKind::PIPE, "|"},
        {TokenKind::CARET, "^"},
        {TokenKind::TILDE, "~"},
        {TokenKind::BANG, "!"},
        {TokenKind::EQUAL, "="},
        {TokenKind::LESS, "<"},
        {TokenKind::GREATER, ">"},
        
        // Compound operators
        {TokenKind::PLUS_PLUS, "++"},
        {TokenKind::MINUS_MINUS, "--"},
        {TokenKind::ARROW, "->"},
        {TokenKind::PLUS_EQUAL, "+="},
        {TokenKind::MINUS_EQUAL, "-="},
        {TokenKind::STAR_EQUAL, "*="},
        {TokenKind::SLASH_EQUAL, "/="},
        {TokenKind::PERCENT_EQUAL, "%="},
        {TokenKind::AMP_EQUAL, "&="},
        {TokenKind::PIPE_EQUAL, "|="},
        {TokenKind::CARET_EQUAL, "^="},
        {TokenKind::EQUAL_EQUAL, "=="},
        {TokenKind::BANG_EQUAL, "!="},
        {TokenKind::LESS_EQUAL, "<="},
        {TokenKind::GREATER_EQUAL, ">="},
        {TokenKind::LESS_LESS, "<<"},
        {TokenKind::GREATER_GREATER, ">>"},
        {TokenKind::LESS_LESS_EQUAL, "<<="},
        {TokenKind::GREATER_GREATER_EQUAL, ">>="},
        {TokenKind::AMP_AMP, "&&"},
        {TokenKind::PIPE_PIPE, "||"},
        
        // Punctuation
        {TokenKind::DOT, "."},
        {TokenKind::COMMA, ","},
        {TokenKind::SEMICOLON, ";"},
        {TokenKind::COLON, ":"},
        {TokenKind::QUESTION, "?"},
        {TokenKind::LEFT_PAREN, "("},
        {TokenKind::RIGHT_PAREN, ")"},
        {TokenKind::LEFT_BRACKET, "["},
        {TokenKind::RIGHT_BRACKET, "]"},
        {TokenKind::LEFT_BRACE, "{"},
        {TokenKind::RIGHT_BRACE, "}"},
        
        // Special
        {TokenKind::UNKNOWN, "UNKNOWN"}
    };
    
    auto it = token_names.find(kind_);
    if (it != token_names.end()) {
        return it->second;
    } else {
        return "UNKNOWN";
    }
}

} // namespace dsLang
