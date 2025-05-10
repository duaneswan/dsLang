/**
 * lexer.h - Lexical Analyzer Interface for dsLang
 * 
 * This file defines the Lexer class which is responsible for converting
 * source code text into a sequence of tokens for parsing.
 */

#ifndef DSLANG_LEXER_H
#define DSLANG_LEXER_H

#include "token.h"
#include <string>

namespace dsLang {

/**
 * Lexer - Lexical analyzer for dsLang
 * 
 * The lexer converts source code text into a sequence of tokens. It provides
 * methods to get the next token and peek at the next token without consuming it.
 */
class Lexer {
public:
    /**
     * Constructor - Initialize the lexer with source code
     * 
     * @param source_code The source code to tokenize
     * @param filename The name of the source file (for error reporting)
     */
    Lexer(const std::string& source_code, const std::string& filename);
    
    /**
     * GetNextToken - Get the next token from the input
     * 
     * @return The next token
     */
    Token GetNextToken();
    
    /**
     * PeekNextToken - Peek at the next token without consuming it
     * 
     * @return The next token
     */
    Token PeekNextToken();
    
    /**
     * GetFilename - Get the name of the source file
     * 
     * @return The source filename
     */
    const std::string& GetFilename() const { return filename_; }
    
private:
    /**
     * SkipWhitespaceAndComments - Skip whitespace and comments in the input
     */
    void SkipWhitespaceAndComments();
    
    /**
     * ScanIdentifierOrKeyword - Scan an identifier or keyword from the input
     * 
     * @return The scanned token
     */
    Token ScanIdentifierOrKeyword();
    
    /**
     * ScanNumber - Scan a numeric literal from the input
     * 
     * @return The scanned token
     */
    Token ScanNumber();
    
    /**
     * ScanString - Scan a string literal from the input
     * 
     * @return The scanned token
     */
    Token ScanString();
    
    /**
     * ScanChar - Scan a character literal from the input
     * 
     * @return The scanned token
     */
    Token ScanChar();
    
    /**
     * ScanOperatorOrPunctuation - Scan an operator or punctuation from the input
     * 
     * @return The scanned token
     */
    Token ScanOperatorOrPunctuation();
    
    /**
     * CreateToken - Create a token with the current line and column
     * 
     * @param kind The token kind
     * @param lexeme The lexeme (exact text in source)
     * @return The created token
     */
    Token CreateToken(TokenKind kind, const std::string& lexeme);
    
    /**
     * ReportError - Report a lexical error
     * 
     * @param message The error message
     */
    void ReportError(const std::string& message);
    
private:
    std::string source_;        // The source code
    std::string filename_;      // The source filename
    size_t current_pos_;        // Current position in the source
    unsigned line_;             // Current line number
    unsigned column_;           // Current column number
    bool peeked_token_;         // Whether we have peeked at a token
    Token next_token_;          // The next token (for peeking)
};

} // namespace dsLang

#endif // DSLANG_LEXER_H
