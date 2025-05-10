/**
 * token.h - Token Definitions for dsLang
 * 
 * This file defines the Token class and token kinds used by the lexical analyzer.
 */

#ifndef DSLANG_TOKEN_H
#define DSLANG_TOKEN_H

#include <string>

namespace dsLang {

/**
 * TokenKind - Enumeration of all token kinds
 */
enum class TokenKind {
    // End of file
    END_OF_FILE,
    
    // Literals
    IDENTIFIER,
    INT_LITERAL,
    FLOAT_LITERAL,
    CHAR_LITERAL,
    STRING_LITERAL,
    
    // Keywords
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_FOR,
    KW_DO,
    KW_BREAK,
    KW_CONTINUE,
    KW_RETURN,
    KW_VOID,
    KW_BOOL,
    KW_CHAR,
    KW_SHORT,
    KW_INT,
    KW_LONG,
    KW_FLOAT,
    KW_DOUBLE,
    KW_UNSIGNED,
    KW_STRUCT,
    KW_ENUM,
    KW_CONST,
    KW_TRUE,
    KW_FALSE,
    KW_NULL,
    
    // Operators
    PLUS,           // +
    MINUS,          // -
    STAR,           // *
    SLASH,          // /
    PERCENT,        // %
    AMP,            // &
    PIPE,           // |
    CARET,          // ^
    TILDE,          // ~
    BANG,           // !
    EQUAL,          // =
    LESS,           // <
    GREATER,        // >
    
    // Compound operators
    PLUS_PLUS,      // ++
    MINUS_MINUS,    // --
    ARROW,          // ->
    PLUS_EQUAL,     // +=
    MINUS_EQUAL,    // -=
    STAR_EQUAL,     // *=
    SLASH_EQUAL,    // /=
    PERCENT_EQUAL,  // %=
    AMP_EQUAL,      // &=
    PIPE_EQUAL,     // |=
    CARET_EQUAL,    // ^=
    EQUAL_EQUAL,    // ==
    BANG_EQUAL,     // !=
    LESS_EQUAL,     // <=
    GREATER_EQUAL,  // >=
    LESS_LESS,      // <<
    GREATER_GREATER, // >>
    LESS_LESS_EQUAL, // <<=
    GREATER_GREATER_EQUAL, // >>=
    AMP_AMP,        // &&
    PIPE_PIPE,      // ||
    
    // Punctuation
    DOT,            // .
    COMMA,          // ,
    SEMICOLON,      // ;
    COLON,          // :
    QUESTION,       // ?
    LEFT_PAREN,     // (
    RIGHT_PAREN,    // )
    LEFT_BRACKET,   // [
    RIGHT_BRACKET,  // ]
    LEFT_BRACE,     // {
    RIGHT_BRACE,    // }
    
    // Special
    UNKNOWN         // Unknown token
};

/**
 * Token - Represents a token in the source code
 */
class Token {
public:
    /**
     * Constructor - Create a token
     * 
     * @param kind The token kind
     * @param lexeme The lexeme (exact text in source)
     * @param value The token value (interpreted text)
     * @param line The line number
     * @param column The column number
     */
    Token(TokenKind kind,
          const std::string& lexeme,
          const std::string& value,
          unsigned line,
          unsigned column)
        : kind_(kind),
          lexeme_(lexeme),
          value_(value),
          line_(line),
          column_(column) {
    }
    
    /**
     * Default constructor
     */
    Token()
        : kind_(TokenKind::UNKNOWN),
          lexeme_(""),
          value_(""),
          line_(0),
          column_(0) {
    }
    
    /**
     * GetKind - Get the token kind
     */
    TokenKind GetKind() const { return kind_; }
    
    /**
     * GetLexeme - Get the token lexeme
     */
    const std::string& GetLexeme() const { return lexeme_; }
    
    /**
     * GetValue - Get the token value
     */
    const std::string& GetValue() const { return value_; }
    
    /**
     * GetLine - Get the token line number
     */
    unsigned GetLine() const { return line_; }
    
    /**
     * GetColumn - Get the token column number
     */
    unsigned GetColumn() const { return column_; }
    
    /**
     * GetTokenName - Get a string representation of the token kind
     */
    std::string GetTokenName() const;
    
private:
    TokenKind kind_;        // The token kind
    std::string lexeme_;    // The exact text from source
    std::string value_;     // The interpreted value
    unsigned line_;         // The line number
    unsigned column_;       // The column number
};

} // namespace dsLang

#endif // DSLANG_TOKEN_H
