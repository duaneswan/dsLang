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
            std::dynamic_pointer_cast<SubscriptExpr>(expr) ||
            std::dynamic_pointer_cast<MemberExpr>(expr)) {
            return std::make_shared<AssignExpr>(expr, value);
        }
        
        ReportError("Invalid assignment target");
    }
    
    return expr;
}

/**
 * ParseLogicalOr - Parse a logical OR expression
 */
std::shared_ptr<Expr> Parser::ParseLogicalOr() {
    auto expr = ParseLogicalAnd();
    
    while (Match(TokenKind::LOGICAL_OR)) {
        auto right = ParseLogicalAnd();
        expr = MakeBinaryExpr(BinaryExpr::Op::LOGICAL_OR, expr, right);
    }
    
    return expr;
}

/**
 * ParseLogicalAnd - Parse a logical AND expression
 */
std::shared_ptr<Expr> Parser::ParseLogicalAnd() {
    auto expr = ParseBitwiseOr();
    
    while (Match(TokenKind::LOGICAL_AND)) {
        auto right = ParseBitwiseOr();
        expr = MakeBinaryExpr(BinaryExpr::Op::LOGICAL_AND, expr, right);
    }
    
    return expr;
}

/**
 * ParseBitwiseOr - Parse a bitwise OR expression
 */
std::shared_ptr<Expr> Parser::ParseBitwiseOr() {
    auto expr = ParseBitwiseXor();
    
    while (Match(TokenKind::PIPE)) {
        auto right = ParseBitwiseXor();
        expr = MakeBinaryExpr(BinaryExpr::Op::BIT_OR, expr, right);
    }
    
    return expr;
}

/**
 * ParseBitwiseXor - Parse a bitwise XOR expression
 */
std::shared_ptr<Expr> Parser::ParseBitwiseXor() {
    auto expr = ParseBitwiseAnd();
    
    while (Match(TokenKind::CARET)) {
        auto right = ParseBitwiseAnd();
        expr = MakeBinaryExpr(BinaryExpr::Op::BIT_XOR, expr, right);
    }
    
    return expr;
}

/**
 * ParseBitwiseAnd - Parse a bitwise AND expression
 */
std::shared_ptr<Expr> Parser::ParseBitwiseAnd() {
    auto expr = ParseEquality();
    
    while (Match(TokenKind::AMPERSAND)) {
        auto right = ParseEquality();
        expr = MakeBinaryExpr(BinaryExpr::Op::BIT_AND, expr, right);
    }
    
    return expr;
}

/**
 * ParseEquality - Parse an equality expression
 */
std::shared_ptr<Expr> Parser::ParseEquality() {
    auto expr = ParseComparison();
    
    while (Match(TokenKind::EQUAL_EQUAL) || Match(TokenKind::BANG_EQUAL)) {
        auto op = (Peek(-1).GetKind() == TokenKind::EQUAL_EQUAL) ? 
                  BinaryExpr::Op::EQ : 
                  BinaryExpr::Op::NE;
        auto right = ParseComparison();
        expr = MakeBinaryExpr(op, expr, right);
    }
    
    return expr;
}

/**
 * ParseComparison - Parse a comparison expression
 */
std::shared_ptr<Expr> Parser::ParseComparison() {
    auto expr = ParseShift();
    
    while (Match(TokenKind::LESS) || Match(TokenKind::LESS_EQUAL) ||
           Match(TokenKind::GREATER) || Match(TokenKind::GREATER_EQUAL)) {
        BinaryExpr::Op op;
        switch (Peek(-1).GetKind()) {
            case TokenKind::LESS:
                op = BinaryExpr::Op::LT;
                break;
            case TokenKind::LESS_EQUAL:
                op = BinaryExpr::Op::LE;
                break;
            case TokenKind::GREATER:
                op = BinaryExpr::Op::GT;
                break;
            case TokenKind::GREATER_EQUAL:
                op = BinaryExpr::Op::GE;
                break;
            default:
                // Unreachable
                op = BinaryExpr::Op::EQ;
                break;
        }
        auto right = ParseShift();
        expr = MakeBinaryExpr(op, expr, right);
    }
    
    return expr;
}

/**
 * ParseShift - Parse a shift expression
 */
std::shared_ptr<Expr> Parser::ParseShift() {
    auto expr = ParseAdditive();
    
    while (Match(TokenKind::LESS_LESS) || Match(TokenKind::GREATER_GREATER)) {
        auto op = (Peek(-1).GetKind() == TokenKind::LESS_LESS) ? 
                  BinaryExpr::Op::SHL : 
                  BinaryExpr::Op::SHR;
        auto right = ParseAdditive();
        expr = MakeBinaryExpr(op, expr, right);
    }
    
    return expr;
}

/**
 * ParseAdditive - Parse an additive expression
 */
std::shared_ptr<Expr> Parser::ParseAdditive() {
    auto expr = ParseMultiplicative();
    
    while (Match(TokenKind::PLUS) || Match(TokenKind::MINUS)) {
        auto op = (Peek(-1).GetKind() == TokenKind::PLUS) ? 
                  BinaryExpr::Op::ADD : 
                  BinaryExpr::Op::SUB;
        auto right = ParseMultiplicative();
        expr = MakeBinaryExpr(op, expr, right);
    }
    
    return expr;
}

/**
 * ParseMultiplicative - Parse a multiplicative expression
 */
std::shared_ptr<Expr> Parser::ParseMultiplicative() {
    auto expr = ParseUnary();
    
    while (Match(TokenKind::STAR) || Match(TokenKind::SLASH) || Match(TokenKind::PERCENT)) {
        BinaryExpr::Op op;
        switch (Peek(-1).GetKind()) {
            case TokenKind::STAR:
                op = BinaryExpr::Op::MUL;
                break;
            case TokenKind::SLASH:
                op = BinaryExpr::Op::DIV;
                break;
            case TokenKind::PERCENT:
                op = BinaryExpr::Op::MOD;
                break;
            default:
                // Unreachable
                op = BinaryExpr::Op::ADD;
                break;
        }
        auto right = ParseUnary();
        expr = MakeBinaryExpr(op, expr, right);
    }
    
    return expr;
}

/**
 * ParseUnary - Parse a unary expression
 */
std::shared_ptr<Expr> Parser::ParseUnary() {
    if (Match(TokenKind::BANG) || Match(TokenKind::MINUS) || 
        Match(TokenKind::TILDE) || Match(TokenKind::STAR) || 
        Match(TokenKind::AMPERSAND)) {
        
        UnaryExpr::Op op;
        switch (Peek(-1).GetKind()) {
            case TokenKind::BANG:
                op = UnaryExpr::Op::NOT;
                break;
            case TokenKind::MINUS:
                op = UnaryExpr::Op::NEG;
                break;
            case TokenKind::TILDE:
                op = UnaryExpr::Op::BIT_NOT;
                break;
            case TokenKind::STAR:
                op = UnaryExpr::Op::DEREF;
                break;
            case TokenKind::AMPERSAND:
                op = UnaryExpr::Op::ADDR_OF;
                break;
            default:
                // Unreachable
                op = UnaryExpr::Op::NOT;
                break;
        }
        
        auto operand = ParseUnary();
        return MakeUnaryExpr(op, operand);
    }
    
    // Handle cast expression
    if (Match(TokenKind::LEFT_PAREN)) {
        // Check if this is a cast expression
        if (Check(TokenKind::KW_VOID) || Check(TokenKind::KW_BOOL) ||
            Check(TokenKind::KW_CHAR) || Check(TokenKind::KW_SHORT) ||
            Check(TokenKind::KW_INT) || Check(TokenKind::KW_LONG) ||
            Check(TokenKind::KW_FLOAT) || Check(TokenKind::KW_DOUBLE) ||
            Check(TokenKind::KW_UNSIGNED) || Check(TokenKind::KW_STRUCT) ||
            Check(TokenKind::KW_ENUM)) {
            
            // This is a cast expression
            Advance(); // Go back to the '('
            return ParseCastExpression();
        }
        
        // This is a grouped expression
        auto expr = ParseExpression();
        Consume(TokenKind::RIGHT_PAREN, "Expected ')' after expression");
        return expr;
    }
    
    return ParsePostfix();
}

/**
 * ParsePostfix - Parse a postfix expression
 */
std::shared_ptr<Expr> Parser::ParsePostfix() {
    auto expr = ParsePrimary();
    
    while (true) {
        if (Match(TokenKind::LEFT_PAREN)) {
            // Function call
            expr = ParseFunctionCall(expr);
        } else if (Match(TokenKind::LEFT_BRACKET)) {
            // Array subscript
            expr = ParseSubscript(expr);
        } else if (Match(TokenKind::DOT)) {
            // Member access
            if (!Check(TokenKind::IDENTIFIER)) {
                ReportError("Expected identifier after '.'");
                return nullptr;
            }
            
            std::string member = Peek().GetLexeme();
            Advance();
            
            expr = std::make_shared<MemberExpr>(expr, member);
        } else if (Match(TokenKind::ARROW)) {
            // Member access through pointer
            if (!Check(TokenKind::IDENTIFIER)) {
                ReportError("Expected identifier after '->'");
                return nullptr;
            }
            
            std::string member = Peek().GetLexeme();
            Advance();
            
            // Create a temporary deref expression
            auto deref = MakeUnaryExpr(UnaryExpr::Op::DEREF, expr);
            expr = std::make_shared<MemberExpr>(deref, member);
        } else {
            break;
        }
    }
    
    return expr;
}

/**
 * ParsePrimary - Parse a primary expression
 */
std::shared_ptr<Expr> Parser::ParsePrimary() {
    if (Match(TokenKind::LEFT_BRACKET)) {
        return ParseMessageExpression();
    }
    
    if (Match(TokenKind::IDENTIFIER)) {
        std::string name = Peek(-1).GetLexeme();
        return std::make_shared<VarExpr>(name);
    }
    
    if (Match(TokenKind::INT_LITERAL)) {
        int64_t value = std::stoll(Peek(-1).GetLexeme());
        return std::make_shared<LiteralExpr>(value);
    }
    
    if (Match(TokenKind::FLOAT_LITERAL)) {
        double value = std::stod(Peek(-1).GetLexeme());
        return std::make_shared<LiteralExpr>(value);
    }
    
    if (Match(TokenKind::CHAR_LITERAL)) {
        char value = Peek(-1).GetLexeme()[0];
        return std::make_shared<LiteralExpr>(value);
    }
    
    if (Match(TokenKind::STRING_LITERAL)) {
        std::string value = Peek(-1).GetLexeme();
        return std::make_shared<LiteralExpr>(value);
    }
    
    if (Match(TokenKind::KW_TRUE)) {
        return std::make_shared<LiteralExpr>(true);
    }
    
    if (Match(TokenKind::KW_FALSE)) {
        return std::make_shared<LiteralExpr>(false);
    }
    
    if (Match(TokenKind::KW_NULL)) {
        return std::make_shared<LiteralExpr>();
    }
    
    ReportError("Expected expression");
    return nullptr;
}

/**
 * ParseMessageExpression - Parse a message expression (Objective-C style)
 */
std::shared_ptr<Expr> Parser::ParseMessageExpression() {
    // Parse the receiver
    auto receiver = ParseExpression();
    
    // Parse the selector and arguments
    if (!Check(TokenKind::IDENTIFIER)) {
        ReportError("Expected selector name in message expression");
        return nullptr;
    }
    
    std::string selector = Peek().GetLexeme();
    Advance();
    
    std::vector<std::shared_ptr<Expr>> arguments;
    
    // Check if we have arguments
    if (Match(TokenKind::COLON)) {
        // This is a method with at least one argument
        do {
            auto arg = ParseExpression();
            arguments.push_back(arg);
            
            // If we have another argument, it should be preceded by an identifier and colon
            if (Check(TokenKind::IDENTIFIER) && CheckNext(TokenKind::COLON)) {
                std::string part_name = Peek().GetLexeme();
                selector += "_" + part_name;
                Advance(); // Consume identifier
                Consume(TokenKind::COLON, "Expected ':' after selector part");
            }
        } while (Check(TokenKind::IDENTIFIER) && CheckNext(TokenKind::COLON));
    }
    
    // Consume the closing bracket
    Consume(TokenKind::RIGHT_BRACKET, "Expected ']' after message expression");
    
    return std::make_shared<MessageExpr>(receiver, selector, arguments);
}

/**
 * ParseFunctionCall - Parse a function call
 */
std::shared_ptr<Expr> Parser::ParseFunctionCall(std::shared_ptr<Expr> callee) {
    std::vector<std::shared_ptr<Expr>> arguments;
    
    if (!Check(TokenKind::RIGHT_PAREN)) {
        do {
            auto arg = ParseExpression();
            arguments.push_back(arg);
        } while (Match(TokenKind::COMMA));
    }
    
    Consume(TokenKind::RIGHT_PAREN, "Expected ')' after function arguments");
    
    return std::make_shared<CallExpr>(callee, arguments);
}

/**
 * ParseSubscript - Parse an array subscript
 */
std::shared_ptr<Expr> Parser::ParseSubscript(std::shared_ptr<Expr> array) {
    auto index = ParseExpression();
    
    Consume(TokenKind::RIGHT_BRACKET, "Expected ']' after array index");
    
    return std::make_shared<SubscriptExpr>(array, index);
}

/**
 * ParseCastExpression - Parse a cast expression
 */
std::shared_ptr<Expr> Parser::ParseCastExpression() {
    Consume(TokenKind::LEFT_PAREN, "Expected '(' at start of cast expression");
    
    auto type = ParseType();
    
    Consume(TokenKind::RIGHT_PAREN, "Expected ')' after cast type");
    
    auto expr = ParseUnary();
    
    return std::make_shared<CastExpr>(type, expr);
}

/**
 * MakeBinaryExpr - Create a binary expression node
 */
std::shared_ptr<BinaryExpr> Parser::MakeBinaryExpr(BinaryExpr::Op op, 
                                                 std::shared_ptr<Expr> left, 
                                                 std::shared_ptr<Expr> right) {
    return std::make_shared<BinaryExpr>(op, left, right);
}

/**
 * MakeUnaryExpr - Create a unary expression node
 */
std::shared_ptr<UnaryExpr> Parser::MakeUnaryExpr(UnaryExpr::Op op, 
                                               std::shared_ptr<Expr> operand) {
    return std::make_shared<UnaryExpr>(op, operand);
}

} // namespace dsLang
