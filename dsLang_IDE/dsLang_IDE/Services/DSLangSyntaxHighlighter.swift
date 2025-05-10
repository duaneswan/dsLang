import Cocoa

class DSLangSyntaxHighlighter {
    
    // MARK: - Token Types
    
    enum TokenType {
        case keyword
        case type
        case identifier
        case number
        case string
        case comment
        case operator
        case punctuation
    }
    
    // MARK: - Constants
    
    // Language keywords
    private let keywords: Set<String> = [
        "define", "as", "with", "parameters", "returns", "and", "it",
        "if", "then", "else", "end", "loop", "while", "for", "in",
        "function", "variable", "constant", "record", "protocol", "module",
        "import", "return", "break", "continue",
        "true", "false", "nil"
    ]
    
    // Primitive types
    private let types: Set<String> = [
        "Int", "Float", "Double", "Bool", "String", "Char", 
        "Void", "Any", "Array", "Dictionary"
    ]
    
    // Operators
    private let operators: Set<String> = [
        "+", "-", "*", "/", "%", "=", "==", "!=", ">", "<", ">=", "<=",
        "&&", "||", "!", "^", "&", "|", "~", "<<", ">>", "+=", "-=", "*=", "/="
    ]
    
    // MARK: - Attributes
    
    private let keywordAttributes: [NSAttributedString.Key: Any] = [
        .foregroundColor: NSColor.systemBlue,
        .font: NSFont.monospacedSystemFont(ofSize: 12, weight: .semibold)
    ]
    
    private let typeAttributes: [NSAttributedString.Key: Any] = [
        .foregroundColor: NSColor.systemPurple,
        .font: NSFont.monospacedSystemFont(ofSize: 12, weight: .semibold)
    ]
    
    private let identifierAttributes: [NSAttributedString.Key: Any] = [
        .foregroundColor: NSColor.textColor,
        .font: NSFont.monospacedSystemFont(ofSize: 12, weight: .regular)
    ]
    
    private let numberAttributes: [NSAttributedString.Key: Any] = [
        .foregroundColor: NSColor.systemOrange,
        .font: NSFont.monospacedSystemFont(ofSize: 12, weight: .regular)
    ]
    
    private let stringAttributes: [NSAttributedString.Key: Any] = [
        .foregroundColor: NSColor.systemRed,
        .font: NSFont.monospacedSystemFont(ofSize: 12, weight: .regular)
    ]
    
    private let commentAttributes: [NSAttributedString.Key: Any] = [
        .foregroundColor: NSColor.systemGreen,
        .font: NSFont.monospacedSystemFont(ofSize: 12, weight: .regular)
    ]
    
    private let operatorAttributes: [NSAttributedString.Key: Any] = [
        .foregroundColor: NSColor.systemBrown,
        .font: NSFont.monospacedSystemFont(ofSize: 12, weight: .regular)
    ]
    
    private let punctuationAttributes: [NSAttributedString.Key: Any] = [
        .foregroundColor: NSColor.systemGray,
        .font: NSFont.monospacedSystemFont(ofSize: 12, weight: .regular)
    ]
    
    // MARK: - Public Methods
    
    /// Applies syntax highlighting to the given text storage for dsLang code
    func highlightSyntax(in textStorage: NSTextStorage, for text: String) {
        // Performance optimization - only process visible range if needed
        // For now, we'll process the entire text
        
        // Create a tokenizer from the text
        let tokens = tokenize(text)
        
        // Apply attributes based on token types
        for token in tokens {
            let attributes = attributesForTokenType(token.type)
            textStorage.addAttributes(attributes, range: token.range)
        }
    }
    
    // MARK: - Tokenization
    
    private struct Token {
        let type: TokenType
        let range: NSRange
    }
    
    private func tokenize(_ text: String) -> [Token] {
        var tokens: [Token] = []
        let nsText = text as NSString
        
        // Simple tokenization using regular expressions
        // In a real implementation, you'd use a more sophisticated lexer
        
        // Comments
        addTokens(forPattern: "//.*?$", in: nsText, type: .comment, to: &tokens)
        addTokens(forPattern: "/\\*.*?\\*/", in: nsText, type: .comment, options: [.dotMatchesLineSeparators], to: &tokens)
        
        // Strings
        addTokens(forPattern: "\".*?\"", in: nsText, type: .string, to: &tokens)
        
        // Numbers
        addTokens(forPattern: "\\b\\d+\\.\\d+\\b", in: nsText, type: .number, to: &tokens) // Float
        addTokens(forPattern: "\\b\\d+\\b", in: nsText, type: .number, to: &tokens) // Int
        
        // Keywords, types, and identifiers
        addTokens(forPattern: "\\b[a-zA-Z_][a-zA-Z0-9_]*\\b", in: nsText, type: .identifier) { word in
            if self.keywords.contains(word) {
                return .keyword
            } else if self.types.contains(word) {
                return .type
            } else {
                return .identifier
            }
        }, to: &tokens)
        
        // Operators
        let operatorPattern = operators.map { NSRegularExpression.escapedPattern(for: $0) }.joined(separator: "|")
        addTokens(forPattern: operatorPattern, in: nsText, type: .operator, to: &tokens)
        
        // Punctuation
        addTokens(forPattern: "[(){}\\[\\],.:;]", in: nsText, type: .punctuation, to: &tokens)
        
        return tokens
    }
    
    private func addTokens(
        forPattern pattern: String,
        in text: NSString,
        type: TokenType,
        options: NSRegularExpression.Options = [],
        to tokens: inout [Token]
    ) {
        addTokens(forPattern: pattern, in: text, type: type, options: options, typeHandler: { _ in type }, to: &tokens)
    }
    
    private func addTokens(
        forPattern pattern: String,
        in text: NSString,
        type: TokenType,
        options: NSRegularExpression.Options = [],
        typeHandler: @escaping (String) -> TokenType,
        to tokens: inout [Token]
    ) {
        do {
            let regex = try NSRegularExpression(pattern: pattern, options: options)
            let fullRange = NSRange(location: 0, length: text.length)
            let matches = regex.matches(in: text as String, options: [], range: fullRange)
            
            for match in matches {
                let matchRange = match.range
                let word = text.substring(with: matchRange)
                let tokenType = typeHandler(word)
                tokens.append(Token(type: tokenType, range: matchRange))
            }
        } catch {
            print("Error creating regex for pattern \(pattern): \(error)")
        }
    }
    
    private func attributesForTokenType(_ type: TokenType) -> [NSAttributedString.Key: Any] {
        switch type {
        case .keyword:
            return keywordAttributes
        case .type:
            return typeAttributes
        case .identifier:
            return identifierAttributes
        case .number:
            return numberAttributes
        case .string:
            return stringAttributes
        case .comment:
            return commentAttributes
        case .operator:
            return operatorAttributes
        case .punctuation:
            return punctuationAttributes
        }
    }
}
