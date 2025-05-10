/**
 * lexer.cpp - Lexical Analyzer Implementation for dsLang
 * 
 * This file implements the Lexer class which is responsible for converting
 * source code text into a sequence of tokens for parsing.
 */

#include "lexer.h"
#include <unordered_map>
#include <cctype>
#include <iostream>
#include <cassert>

namespace dsLang {

// Static keyword map for fast keyword lookup
static const std::unordered_map<std::string, TokenKind> keywords = {
    {"if", TokenKind::KW_IF},
    {"else", TokenKind::KW_ELSE},
    {"while", TokenKind::KW_WHILE},
    {"for", TokenKind::KW_FOR},
    {"do", TokenKind::KW_DO},
    {"break", TokenKind::KW_BREAK},
    {"continue", TokenKind::KW_CONTINUE},
    {"return", TokenKind::KW_RETURN},
    {"void", TokenKind::KW_VOID},
    {"bool", TokenKind::KW_BOOL},
    {"char", TokenKind::KW_CHAR},
    {"short", TokenKind::KW_SHORT},
    {"int", TokenKind::KW_INT},
    {"long", TokenKind::KW_LONG},
    {"float", TokenKind::KW_FLOAT},
    {"double", TokenKind::KW_DOUBLE},
    {"unsigned", TokenKind::KW_UNSIGNED},
    {"struct", TokenKind::KW_STRUCT},
    {"enum", TokenKind::KW_ENUM},
    {"const", TokenKind::KW_CONST},
    {"true", TokenKind::KW_TRUE},
    {"false", TokenKind::KW_FALSE},
    {"null", TokenKind::KW_NULL}
};

/**
 * Constructor - Initialize the lexer with source code
 */
Lexer::Lexer(const std::string& source_code, const std::string& filename)
    : source_(source_code),
      filename_(filename),
      current_pos_(0),
      line_(1),
      column_(1),
      peeked_token_(false) {
}

/**
 * GetNextToken - Get the next token from the input
 */
Token Lexer::GetNextToken() {
    if (peeked_token_) {
        peeked_token_ = false;
        return next_token_;
    }
    
    SkipWhitespaceAndComments();
    
    if (current_pos_ >= source_.size()) {
        return CreateToken(TokenKind::END_OF_FILE, "");
    }
    
    char c = source_[current_pos_];
    
    // Identifier or keyword
    if (std::isalpha(c) || c == '_') {
        return ScanIdentifierOrKeyword();
    }
    
    // Number
    if (std::isdigit(c)) {
        return ScanNumber();
    }
    
    // String literal
    if (c == '"') {
        return ScanString();
    }
    
    // Character literal
    if (c == '\'') {
        return ScanChar();
    }
    
    // Operator or punctuation
    return ScanOperatorOrPunctuation();
}

/**
 * PeekNextToken - Peek at the next token without consuming it
 */
Token Lexer::PeekNextToken() {
    if (!peeked_token_) {
        next_token_ = GetNextToken();
        peeked_token_ = true;
    }
    return next_token_;
}

/**
 * SkipWhitespaceAndComments - Skip whitespace and comments in the input
 */
void Lexer::SkipWhitespaceAndComments() {
    while (current_pos_ < source_.size()) {
        char c = source_[current_pos_];
        
        // Skip whitespace
        if (std::isspace(c)) {
            if (c == '\n') {
                line_++;
                column_ = 1;
            } else {
                column_++;
            }
            current_pos_++;
            continue;
        }
        
        // Skip single-line comment
        if (c == '/' && current_pos_ + 1 < source_.size() && source_[current_pos_ + 1] == '/') {
            current_pos_ += 2;
            column_ += 2;
            
            while (current_pos_ < source_.size() && source_[current_pos_] != '\n') {
                current_pos_++;
                column_++;
            }
            continue;
        }
        
        // Skip multi-line comment
        if (c == '/' && current_pos_ + 1 < source_.size() && source_[current_pos_ + 1] == '*') {
            current_pos_ += 2;
            column_ += 2;
            
            while (current_pos_ + 1 < source_.size() &&
                   !(source_[current_pos_] == '*' && source_[current_pos_ + 1] == '/')) {
                if (source_[current_pos_] == '\n') {
                    line_++;
                    column_ = 1;
                } else {
                    column_++;
                }
                current_pos_++;
            }
            
            if (current_pos_ + 1 < source_.size()) {
                current_pos_ += 2;  // Skip */
                column_ += 2;
            } else {
                // End of file in multi-line comment
                ReportError("Unterminated multi-line comment");
            }
            continue;
        }
        
        // Not whitespace or comment, so we're done skipping
        break;
    }
}

/**
 * ScanIdentifierOrKeyword - Scan an identifier or keyword from the input
 */
Token Lexer::ScanIdentifierOrKeyword() {
    size_t start_pos = current_pos_;
    unsigned start_column = column_;
    
    while (current_pos_ < source_.size() &&
           (std::isalnum(source_[current_pos_]) || source_[current_pos_] == '_')) {
        current_pos_++;
        column_++;
    }
    
    std::string lexeme = source_.substr(start_pos, current_pos_ - start_pos);
    
    // Check if it's a keyword
    auto it = keywords.find(lexeme);
    if (it != keywords.end()) {
        return Token(it->second, lexeme, lexeme, line_, start_column);
    }
    
    // It's an identifier
    return Token(TokenKind::IDENTIFIER, lexeme, lexeme, line_, start_column);
}

/**
 * ScanNumber - Scan a numeric literal from the input
 */
Token Lexer::ScanNumber() {
    size_t start_pos = current_pos_;
    unsigned start_column = column_;
    bool is_float = false;
    
    // Check if it's a hex number
    if (current_pos_ + 1 < source_.size() && 
        source_[current_pos_] == '0' && 
        (source_[current_pos_ + 1] == 'x' || source_[current_pos_ + 1] == 'X')) {
        current_pos_ += 2;  // Skip 0x
        column_ += 2;
        
        if (current_pos_ >= source_.size() || !std::isxdigit(source_[current_pos_])) {
            ReportError("Invalid hexadecimal literal");
        }
        
        while (current_pos_ < source_.size() && std::isxdigit(source_[current_pos_])) {
            current_pos_++;
            column_++;
        }
    }
    // Decimal or octal number
    else {
        // Scan integer part
        while (current_pos_ < source_.size() && std::isdigit(source_[current_pos_])) {
            current_pos_++;
            column_++;
        }
        
        // Scan fractional part if exists
        if (current_pos_ < source_.size() && source_[current_pos_] == '.') {
            is_float = true;
            current_pos_++;  // Skip .
            column_++;
            
            while (current_pos_ < source_.size() && std::isdigit(source_[current_pos_])) {
                current_pos_++;
                column_++;
            }
        }
        
        // Scan exponent if exists
        if (current_pos_ < source_.size() && 
            (source_[current_pos_] == 'e' || source_[current_pos_] == 'E')) {
            is_float = true;
            current_pos_++;  // Skip e/E
            column_++;
            
            // Check for +/- in exponent
            if (current_pos_ < source_.size() && 
                (source_[current_pos_] == '+' || source_[current_pos_] == '-')) {
                current_pos_++;
                column_++;
            }
            
            if (current_pos_ >= source_.size() || !std::isdigit(source_[current_pos_])) {
                ReportError("Invalid floating-point literal exponent");
            }
            
            while (current_pos_ < source_.size() && std::isdigit(source_[current_pos_])) {
                current_pos_++;
                column_++;
            }
        }
        
        // Check for float suffix
        if (current_pos_ < source_.size() && 
            (source_[current_pos_] == 'f' || source_[current_pos_] == 'F')) {
            is_float = true;
            current_pos_++;
            column_++;
        }
    }
    
    std::string lexeme = source_.substr(start_pos, current_pos_ - start_pos);
    TokenKind kind = is_float ? TokenKind::FLOAT_LITERAL : TokenKind::INT_LITERAL;
    
    return Token(kind, lexeme, lexeme, line_, start_column);
}

/**
 * ScanString - Scan a string literal from the input
 */
Token Lexer::ScanString() {
    size_t start_pos = current_pos_;
    unsigned start_column = column_;
    std::string value;
    
    current_pos_++;  // Skip "
    column_++;
    
    while (current_pos_ < source_.size() && source_[current_pos_] != '"') {
        char c = source_[current_pos_];
        
        if (c == '\n') {
            ReportError("Unterminated string literal");
        }
        
        // Handle escape sequences
        if (c == '\\' && current_pos_ + 1 < source_.size()) {
            current_pos_++;  // Skip \
            column_++;
            c = source_[current_pos_];
            
            switch (c) {
                case 'n': value += '\n'; break;
                case 'r': value += '\r'; break;
                case 't': value += '\t'; break;
                case '\\': value += '\\'; break;
                case '"': value += '"'; break;
                case '\'': value += '\''; break;
                default:
                    ReportError("Invalid escape sequence in string literal");
            }
        } else {
            value += c;
        }
        
        current_pos_++;
        column_++;
    }
    
    if (current_pos_ >= source_.size()) {
        ReportError("Unterminated string literal");
    }
    
    current_pos_++;  // Skip closing "
    column_++;
    
    std::string lexeme = source_.substr(start_pos, current_pos_ - start_pos);
    return Token(TokenKind::STRING_LITERAL, lexeme, value, line_, start_column);
}

/**
 * ScanChar - Scan a character literal from the input
 */
Token Lexer::ScanChar() {
    size_t start_pos = current_pos_;
    unsigned start_column = column_;
    char value;
    
    current_pos_++;  // Skip '
    column_++;
    
    if (current_pos_ >= source_.size()) {
        ReportError("Unterminated character literal");
    }
    
    // Handle escape sequences
    if (source_[current_pos_] == '\\') {
        current_pos_++;  // Skip \
        column_++;
        
        if (current_pos_ >= source_.size()) {
            ReportError("Unterminated character literal");
        }
        
        char c = source_[current_pos_];
        switch (c) {
            case 'n': value = '\n'; break;
            case 'r': value = '\r'; break;
            case 't': value = '\t'; break;
            case '\\': value = '\\'; break;
            case '\'': value = '\''; break;
            case '"': value = '"'; break;
            default:
                ReportError("Invalid escape sequence in character literal");
                value = c;  // Default value
        }
    } else {
        value = source_[current_pos_];
    }
    
    current_pos_++;  // Skip character
    column_++;
    
    if (current_pos_ >= source_.size() || source_[current_pos_] != '\'') {
        ReportError("Unterminated character literal");
    }
    
    current_pos_++;  // Skip closing '
    column_++;
    
    std::string lexeme = source_.substr(start_pos, current_pos_ - start_pos);
    return Token(TokenKind::CHAR_LITERAL, lexeme, std::string(1, value), line_, start_column);
}

/**
 * ScanOperatorOrPunctuation - Scan an operator or punctuation from the input
 */
Token Lexer::ScanOperatorOrPunctuation() {
    size_t start_pos = current_pos_;
    unsigned start_column = column_;
    
    char c = source_[current_pos_++];
    column_++;
    TokenKind kind;
    
    switch (c) {
        case '+':
            if (current_pos_ < source_.size()) {
                if (source_[current_pos_] == '+') {
                    current_pos_++;
                    column_++;
                    return Token(TokenKind::PLUS_PLUS, "++", "++", line_, start_column);
                } else if (source_[current_pos_] == '=') {
                    current_pos_++;
                    column_++;
                    return Token(TokenKind::PLUS_EQUAL, "+=", "+=", line_, start_column);
                }
            }
            return Token(TokenKind::PLUS, "+", "+", line_, start_column);
        
        case '-':
            if (current_pos_ < source_.size()) {
                if (source_[current_pos_] == '-') {
                    current_pos_++;
                    column_++;
                    return Token(TokenKind::MINUS_MINUS, "--", "--", line_, start_column);
                } else if (source_[current_pos_] == '=') {
                    current_pos_++;
                    column_++;
                    return Token(TokenKind::MINUS_EQUAL, "-=", "-=", line_, start_column);
                } else if (source_[current_pos_] == '>') {
                    current_pos_++;
                    column_++;
                    return Token(TokenKind::ARROW, "->", "->", line_, start_column);
                }
            }
            return Token(TokenKind::MINUS, "-", "-", line_, start_column);
        
        case '*':
            if (current_pos_ < source_.size() && source_[current_pos_] == '=') {
                current_pos_++;
                column_++;
                return Token(TokenKind::STAR_EQUAL, "*=", "*=", line_, start_column);
            }
            return Token(TokenKind::STAR, "*", "*", line_, start_column);
        
        case '/':
            if (current_pos_ < source_.size() && source_[current_pos_] == '=') {
                current_pos_++;
                column_++;
                return Token(TokenKind::SLASH_EQUAL, "/=", "/=", line_, start_column);
            }
            return Token(TokenKind::SLASH, "/", "/", line_, start_column);
        
        case '%':
            if (current_pos_ < source_.size() && source_[current_pos_] == '=') {
                current_pos_++;
                column_++;
                return Token(TokenKind::PERCENT_EQUAL, "%=", "%=", line_, start_column);
            }
            return Token(TokenKind::PERCENT, "%", "%", line_, start_column);
        
        case '&':
            if (current_pos_ < source_.size()) {
                if (source_[current_pos_] == '&') {
                    current_pos_++;
                    column_++;
                    return Token(TokenKind::AMP_AMP, "&&", "&&", line_, start_column);
                } else if (source_[current_pos_] == '=') {
                    current_pos_++;
                    column_++;
                    return Token(TokenKind::AMP_EQUAL, "&=", "&=", line_, start_column);
                }
            }
            return Token(TokenKind::AMP, "&", "&", line_, start_column);
        
        case '|':
            if (current_pos_ < source_.size()) {
                if (source_[current_pos_] == '|') {
                    current_pos_++;
                    column_++;
                    return Token(TokenKind::PIPE_PIPE, "||", "||", line_, start_column);
                } else if (source_[current_pos_] == '=') {
                    current_pos_++;
                    column_++;
                    return Token(TokenKind::PIPE_EQUAL, "|=", "|=", line_, start_column);
                }
            }
            return Token(TokenKind::PIPE, "|", "|", line_, start_column);
        
        case '^':
            if (current_pos_ < source_.size() && source_[current_pos_] == '=') {
                current_pos_++;
                column_++;
                return Token(TokenKind::CARET_EQUAL, "^=", "^=", line_, start_column);
            }
            return Token(TokenKind::CARET, "^", "^", line_, start_column);
        
        case '~':
            return Token(TokenKind::TILDE, "~", "~", line_, start_column);
        
        case '!':
            if (current_pos_ < source_.size() && source_[current_pos_] == '=') {
                current_pos_++;
                column_++;
                return Token(TokenKind::BANG_EQUAL, "!=", "!=", line_, start_column);
            }
            return Token(TokenKind::BANG, "!", "!", line_, start_column);
        
        case '=':
            if (current_pos_ < source_.size() && source_[current_pos_] == '=') {
                current_pos_++;
                column_++;
                return Token(TokenKind::EQUAL_EQUAL, "==", "==", line_, start_column);
            }
            return Token(TokenKind::EQUAL, "=", "=", line_, start_column);
        
        case '<':
            if (current_pos_ < source_.size()) {
                if (source_[current_pos_] == '=') {
                    current_pos_++;
                    column_++;
                    return Token(TokenKind::LESS_EQUAL, "<=", "<=", line_, start_column);
                } else if (source_[current_pos_] == '<') {
                    current_pos_++;
                    column_++;
                    if (current_pos_ < source_.size() && source_[current_pos_] == '=') {
                        current_pos_++;
                        column_++;
                        return Token(TokenKind::LESS_LESS_EQUAL, "<<=", "<<=", line_, start_column);
                    }
                    return Token(TokenKind::LESS_LESS, "<<", "<<", line_, start_column);
                }
            }
            return Token(TokenKind::LESS, "<", "<", line_, start_column);
        
        case '>':
            if (current_pos_ < source_.size()) {
                if (source_[current_pos_] == '=') {
                    current_pos_++;
                    column_++;
                    return Token(TokenKind::GREATER_EQUAL, ">=", ">=", line_, start_column);
                } else if (source_[current_pos_] == '>') {
                    current_pos_++;
                    column_++;
                    if (current_pos_ < source_.size() && source_[current_pos_] == '=') {
                        current_pos_++;
                        column_++;
                        return Token(TokenKind::GREATER_GREATER_EQUAL, ">>=", ">>=", line_, start_column);
                    }
                    return Token(TokenKind::GREATER_GREATER, ">>", ">>", line_, start_column);
                }
            }
            return Token(TokenKind::GREATER, ">", ">", line_, start_column);
        
        case '.':
            return Token(TokenKind::DOT, ".", ".", line_, start_column);
        
        case ',':
            return Token(TokenKind::COMMA, ",", ",", line_, start_column);
        
        case ';':
            return Token(TokenKind::SEMICOLON, ";", ";", line_, start_column);
        
        case ':':
            return Token(TokenKind::COLON, ":", ":", line_, start_column);
        
        case '?':
            return Token(TokenKind::QUESTION, "?", "?", line_, start_column);
        
        case '(':
            return Token(TokenKind::LEFT_PAREN, "(", "(", line_, start_column);
        
        case ')':
            return Token(TokenKind::RIGHT_PAREN, ")", ")", line_, start_column);
        
        case '[':
            return Token(TokenKind::LEFT_BRACKET, "[", "[", line_, start_column);
        
        case ']':
            return Token(TokenKind::RIGHT_BRACKET, "]", "]", line_, start_column);
        
        case '{':
            return Token(TokenKind::LEFT_BRACE, "{", "{", line_, start_column);
        
        case '}':
            return Token(TokenKind::RIGHT_BRACE, "}", "}", line_, start_column);
        
        default:
            return Token(TokenKind::UNKNOWN, std::string(1, c), std::string(1, c), line_, start_column);
    }
}

/**
 * CreateToken - Create a token with the current line and column
 */
Token Lexer::CreateToken(TokenKind kind, const std::string& lexeme) {
    return Token(kind, lexeme, lexeme, line_, column_ - lexeme.size());
}

/**
 * ReportError - Report a lexical error
 */
void Lexer::ReportError(const std::string& message) {
    std::cerr << filename_ << ":" << line_ << ":" << column_ << ": error: " << message << std::endl;
    
    // Find the start of the current line
    size_t line_start = current_pos_;
    while (line_start > 0 && source_[line_start - 1] != '\n') {
        line_start--;
    }
    
    // Find the end of the current line
    size_t line_end = current_pos_;
    while (line_end < source_.size() && source_[line_end] != '\n') {
        line_end++;
    }
    
    // Print the line with the error
    std::cerr << source_.substr(line_start, line_end - line_start) << std::endl;
    
    // Print a marker pointing to the error position
    std::cerr << std::string(column_ - 1, ' ') << "^" << std::endl;
    
    // For simplicity, we'll just continue lexing after reporting the error
}

} // namespace dsLang
