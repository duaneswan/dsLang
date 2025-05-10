/**
 * ast.h - Abstract Syntax Tree for dsLang
 * 
 * This file defines the AST (Abstract Syntax Tree) nodes used to represent
 * the structure of a dsLang program.
 */

#ifndef DSLANG_AST_H
#define DSLANG_AST_H

#include <memory>
#include <string>
#include <vector>

namespace dsLang {

// Forward declarations
class ASTVisitor;
class Type;

/**
 * Node - Base class for all AST nodes
 */
class Node {
public:
    virtual ~Node() = default;
    
    /**
     * Accept - Accept a visitor to this node (Visitor pattern)
     */
    virtual void Accept(ASTVisitor* visitor) = 0;
};

//===----------------------------------------------------------------------===//
// Expressions
//===----------------------------------------------------------------------===//

/**
 * Expr - Base class for all expression nodes
 */
class Expr : public Node {
public:
    virtual ~Expr() = default;
    
    /**
     * GetType - Get the type of the expression
     */
    virtual std::shared_ptr<Type> GetType() const = 0;
};

/**
 * BinaryExpr - Binary expression (e.g., a + b, a < b)
 */
class BinaryExpr : public Expr {
public:
    enum class Op {
        ADD,            // +
        SUB,            // -
        MUL,            // *
        DIV,            // /
        MOD,            // %
        BIT_AND,        // &
        BIT_OR,         // |
        BIT_XOR,        // ^
        SHIFT_LEFT,     // <<
        SHIFT_RIGHT,    // >>
        EQUAL,          // ==
        NOT_EQUAL,      // !=
        LESS,           // <
        GREATER,        // >
        LESS_EQUAL,     // <=
        GREATER_EQUAL,  // >=
        LOGICAL_AND,    // &&
        LOGICAL_OR      // ||
    };
    
    /**
     * Constructor
     */
    BinaryExpr(Op op, std::shared_ptr<Expr> left, std::shared_ptr<Expr> right, std::shared_ptr<Type> type)
        : op_(op), left_(left), right_(right), type_(type) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetOp - Get the operator
     */
    Op GetOp() const { return op_; }
    
    /**
     * GetLeft - Get the left operand
     */
    std::shared_ptr<Expr> GetLeft() const { return left_; }
    
    /**
     * GetRight - Get the right operand
     */
    std::shared_ptr<Expr> GetRight() const { return right_; }
    
    /**
     * GetType - Get the type of the expression
     */
    std::shared_ptr<Type> GetType() const override { return type_; }
    
private:
    Op op_;                        // The operator
    std::shared_ptr<Expr> left_;   // The left operand
    std::shared_ptr<Expr> right_;  // The right operand
    std::shared_ptr<Type> type_;   // The result type
};

/**
 * UnaryExpr - Unary expression (e.g., -a, !a, ++a, a++)
 */
class UnaryExpr : public Expr {
public:
    enum class Op {
        NEGATE,         // -
        NOT,            // ~
        LOGICAL_NOT,    // !
        PRE_INC,        // ++a
        PRE_DEC,        // --a
        POST_INC,       // a++
        POST_DEC,       // a--
        ADDR,           // &
        DEREF           // *
    };
    
    /**
     * Constructor
     */
    UnaryExpr(Op op, std::shared_ptr<Expr> operand, std::shared_ptr<Type> type)
        : op_(op), operand_(operand), type_(type) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetOp - Get the operator
     */
    Op GetOp() const { return op_; }
    
    /**
     * GetOperand - Get the operand
     */
    std::shared_ptr<Expr> GetOperand() const { return operand_; }
    
    /**
     * GetType - Get the type of the expression
     */
    std::shared_ptr<Type> GetType() const override { return type_; }
    
private:
    Op op_;                       // The operator
    std::shared_ptr<Expr> operand_; // The operand
    std::shared_ptr<Type> type_;  // The result type
};

/**
 * LiteralExpr - Literal expression (e.g., 42, 3.14, 'a', "hello")
 */
class LiteralExpr : public Expr {
public:
    enum class Kind {
        BOOL,
        INT,
        FLOAT,
        CHAR,
        STRING,
        NULL_PTR
    };
    
    /**
     * Constructor for bool literals
     */
    LiteralExpr(bool value, std::shared_ptr<Type> type)
        : kind_(Kind::BOOL), bool_value_(value), type_(type) {}
    
    /**
     * Constructor for int literals
     */
    LiteralExpr(int64_t value, std::shared_ptr<Type> type)
        : kind_(Kind::INT), int_value_(value), type_(type) {}
    
    /**
     * Constructor for float literals
     */
    LiteralExpr(double value, std::shared_ptr<Type> type)
        : kind_(Kind::FLOAT), float_value_(value), type_(type) {}
    
    /**
     * Constructor for char literals
     */
    LiteralExpr(char value, std::shared_ptr<Type> type)
        : kind_(Kind::CHAR), char_value_(value), type_(type) {}
    
    /**
     * Constructor for string literals
     */
    LiteralExpr(const std::string& value, std::shared_ptr<Type> type)
        : kind_(Kind::STRING), string_value_(value), type_(type) {}
    
    /**
     * Constructor for null pointer literals
     */
    LiteralExpr(std::shared_ptr<Type> type)
        : kind_(Kind::NULL_PTR), type_(type) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetLiteralKind - Get the kind of literal
     */
    Kind GetLiteralKind() const { return kind_; }
    
    /**
     * GetBoolValue - Get the bool value (for bool literals)
     */
    bool GetBoolValue() const { return bool_value_; }
    
    /**
     * GetIntValue - Get the int value (for int literals)
     */
    int64_t GetIntValue() const { return int_value_; }
    
    /**
     * GetFloatValue - Get the float value (for float literals)
     */
    double GetFloatValue() const { return float_value_; }
    
    /**
     * GetCharValue - Get the char value (for char literals)
     */
    char GetCharValue() const { return char_value_; }
    
    /**
     * GetStringValue - Get the string value (for string literals)
     */
    const std::string& GetStringValue() const { return string_value_; }
    
    /**
     * GetType - Get the type of the expression
     */
    std::shared_ptr<Type> GetType() const override { return type_; }
    
private:
    Kind kind_;                   // The kind of literal
    union {
        bool bool_value_;         // Bool value
        int64_t int_value_;       // Int value
        double float_value_;      // Float value
        char char_value_;         // Char value
    };
    std::string string_value_;    // String value
    std::shared_ptr<Type> type_;  // The type
};

/**
 * VarExpr - Variable reference expression (e.g., x)
 */
class VarExpr : public Expr {
public:
    /**
     * Constructor
     */
    VarExpr(const std::string& name, std::shared_ptr<Type> type)
        : name_(name), type_(type) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetName - Get the variable name
     */
    const std::string& GetName() const { return name_; }
    
    /**
     * GetType - Get the type of the expression
     */
    std::shared_ptr<Type> GetType() const override { return type_; }
    
private:
    std::string name_;            // The variable name
    std::shared_ptr<Type> type_;  // The type
};

/**
 * AssignExpr - Assignment expression (e.g., x = 42)
 */
class AssignExpr : public Expr {
public:
    /**
     * Constructor
     */
    AssignExpr(std::shared_ptr<Expr> target, std::shared_ptr<Expr> value)
        : target_(target), value_(value) {}

    /**
     * Constructor with type
     */
    AssignExpr(std::shared_ptr<Expr> target, std::shared_ptr<Expr> value, std::shared_ptr<Type> type)
        : target_(target), value_(value), type_(type) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetTarget - Get the assignment target
     */
    std::shared_ptr<Expr> GetTarget() const { return target_; }
    
    /**
     * GetValue - Get the assigned value
     */
    std::shared_ptr<Expr> GetValue() const { return value_; }
    
    /**
     * GetType - Get the type of the expression
     */
    std::shared_ptr<Type> GetType() const override { 
        return type_ ? type_ : target_->GetType(); 
    }
    
private:
    std::shared_ptr<Expr> target_; // The assignment target
    std::shared_ptr<Expr> value_;  // The assigned value
    std::shared_ptr<Type> type_;   // Optional explicit type
};

/**
 * CallExpr - Function call expression (e.g., foo(a, b))
 */
class CallExpr : public Expr {
public:
    /**
     * Constructor
     */
    CallExpr(const std::string& callee, 
             std::vector<std::shared_ptr<Expr>> args,
             std::shared_ptr<Type> return_type)
        : callee_(callee), args_(args), return_type_(return_type) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetCallee - Get the function name
     */
    const std::string& GetCallee() const { return callee_; }
    
    /**
     * GetArgs - Get the arguments
     */
    const std::vector<std::shared_ptr<Expr>>& GetArgs() const { return args_; }
    
    /**
     * GetType - Get the type of the expression
     */
    std::shared_ptr<Type> GetType() const override { return return_type_; }
    
private:
    std::string callee_;                         // The function name
    std::vector<std::shared_ptr<Expr>> args_;    // The arguments
    std::shared_ptr<Type> return_type_;          // The return type
};

/**
 * MessageExpr - Objective-C style message expression (e.g., [obj foo:a bar:b])
 */
class MessageExpr : public Expr {
public:
    /**
     * Constructor
     */
    MessageExpr(std::shared_ptr<Expr> receiver, 
                const std::string& selector,
                std::vector<std::shared_ptr<Expr>> args,
                std::shared_ptr<Type> return_type)
        : receiver_(receiver), selector_(selector), args_(args), return_type_(return_type) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetReceiver - Get the receiver object
     */
    std::shared_ptr<Expr> GetReceiver() const { return receiver_; }
    
    /**
     * GetSelector - Get the selector (method name)
     */
    const std::string& GetSelector() const { return selector_; }
    
    /**
     * GetArgs - Get the arguments
     */
    const std::vector<std::shared_ptr<Expr>>& GetArgs() const { return args_; }
    
    /**
     * GetType - Get the type of the expression
     */
    std::shared_ptr<Type> GetType() const override { return return_type_; }
    
private:
    std::shared_ptr<Expr> receiver_;             // The receiver object
    std::string selector_;                       // The selector (method name)
    std::vector<std::shared_ptr<Expr>> args_;    // The arguments
    std::shared_ptr<Type> return_type_;          // The return type
};

/**
 * SubscriptExpr - Array subscript expression (e.g., arr[i])
 */
class SubscriptExpr : public Expr {
public:
    /**
     * Constructor
     */
    SubscriptExpr(std::shared_ptr<Expr> array, 
                  std::shared_ptr<Expr> index,
                  std::shared_ptr<Type> elem_type)
        : array_(array), index_(index), elem_type_(elem_type) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetArray - Get the array expression
     */
    std::shared_ptr<Expr> GetArray() const { return array_; }
    
    /**
     * GetIndex - Get the index expression
     */
    std::shared_ptr<Expr> GetIndex() const { return index_; }
    
    /**
     * GetType - Get the type of the expression
     */
    std::shared_ptr<Type> GetType() const override { return elem_type_; }
    
private:
    std::shared_ptr<Expr> array_;   // The array expression
    std::shared_ptr<Expr> index_;   // The index expression
    std::shared_ptr<Type> elem_type_; // The element type
};

/**
 * CastExpr - Type cast expression (e.g., (int)x)
 */
class CastExpr : public Expr {
public:
    /**
     * Constructor
     */
    CastExpr(std::shared_ptr<Expr> expr, std::shared_ptr<Type> type)
        : expr_(expr), type_(type) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetExpr - Get the expression being cast
     */
    std::shared_ptr<Expr> GetExpr() const { return expr_; }
    
    /**
     * GetType - Get the type of the expression
     */
    std::shared_ptr<Type> GetType() const override { return type_; }
    
private:
    std::shared_ptr<Expr> expr_;    // The expression being cast
    std::shared_ptr<Type> type_;    // The target type
};

//===----------------------------------------------------------------------===//
// Statements
//===----------------------------------------------------------------------===//

/**
 * Stmt - Base class for all statement nodes
 */
class Stmt : public Node {
public:
    virtual ~Stmt() = default;
};

/**
 * ExprStmt - Expression statement (e.g., foo(a, b);)
 */
class ExprStmt : public Stmt {
public:
    /**
     * Constructor
     */
    ExprStmt(std::shared_ptr<Expr> expr)
        : expr_(expr) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetExpr - Get the expression
     */
    std::shared_ptr<Expr> GetExpr() const { return expr_; }
    
private:
    std::shared_ptr<Expr> expr_;   // The expression
};

/**
 * BlockStmt - Block statement (e.g., { stmt1; stmt2; })
 */
class BlockStmt : public Stmt {
public:
    /**
     * Constructor
     */
    BlockStmt(std::vector<std::shared_ptr<Stmt>> stmts)
        : stmts_(stmts) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetStmts - Get the statements in the block
     */
    const std::vector<std::shared_ptr<Stmt>>& GetStmts() const { return stmts_; }
    
private:
    std::vector<std::shared_ptr<Stmt>> stmts_;  // The statements in the block
};

/**
 * IfStmt - If statement (e.g., if (cond) { ... } else { ... })
 */
class IfStmt : public Stmt {
public:
    /**
     * Constructor
     */
    IfStmt(std::shared_ptr<Expr> cond,
           std::shared_ptr<Stmt> then_stmt,
           std::shared_ptr<Stmt> else_stmt = nullptr)
        : cond_(cond), then_(then_stmt), else_(else_stmt) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetCond - Get the condition expression
     */
    std::shared_ptr<Expr> GetCond() const { return cond_; }
    
    /**
     * GetThen - Get the then statement
     */
    std::shared_ptr<Stmt> GetThen() const { return then_; }
    
    /**
     * GetElse - Get the else statement
     */
    std::shared_ptr<Stmt> GetElse() const { return else_; }
    
private:
    std::shared_ptr<Expr> cond_;  // The condition expression
    std::shared_ptr<Stmt> then_;  // The then statement
    std::shared_ptr<Stmt> else_;  // The else statement
};

/**
 * WhileStmt - While statement (e.g., while (cond) { ... })
 */
class WhileStmt : public Stmt {
public:
    /**
     * Constructor
     */
    WhileStmt(std::shared_ptr<Expr> cond,
              std::shared_ptr<Stmt> body)
        : cond_(cond), body_(body) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetCond - Get the condition expression
     */
    std::shared_ptr<Expr> GetCond() const { return cond_; }
    
    /**
     * GetBody - Get the body statement
     */
    std::shared_ptr<Stmt> GetBody() const { return body_; }
    
private:
    std::shared_ptr<Expr> cond_;  // The condition expression
    std::shared_ptr<Stmt> body_;  // The body statement
};

/**
 * ForStmt - For statement (e.g., for (init; cond; inc) { ... })
 */
class ForStmt : public Stmt {
public:
    /**
     * Constructor
     */
    ForStmt(std::shared_ptr<Stmt> init,
            std::shared_ptr<Expr> cond,
            std::shared_ptr<Expr> inc,
            std::shared_ptr<Stmt> body)
        : init_(init), cond_(cond), inc_(inc), body_(body) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetInit - Get the initialization statement
     */
    std::shared_ptr<Stmt> GetInit() const { return init_; }
    
    /**
     * GetCond - Get the condition expression
     */
    std::shared_ptr<Expr> GetCond() const { return cond_; }
    
    /**
     * GetInc - Get the increment expression
     */
    std::shared_ptr<Expr> GetInc() const { return inc_; }
    
    /**
     * GetBody - Get the body statement
     */
    std::shared_ptr<Stmt> GetBody() const { return body_; }
    
private:
    std::shared_ptr<Stmt> init_;  // The initialization statement
    std::shared_ptr<Expr> cond_;  // The condition expression
    std::shared_ptr<Expr> inc_;   // The increment expression
    std::shared_ptr<Stmt> body_;  // The body statement
};

/**
 * BreakStmt - Break statement (e.g., break;)
 */
class BreakStmt : public Stmt {
public:
    /**
     * Constructor
     */
    BreakStmt() {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
};

/**
 * ContinueStmt - Continue statement (e.g., continue;)
 */
class ContinueStmt : public Stmt {
public:
    /**
     * Constructor
     */
    ContinueStmt() {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
};

/**
 * ReturnStmt - Return statement (e.g., return x;)
 */
class ReturnStmt : public Stmt {
public:
    /**
     * Constructor
     */
    ReturnStmt(std::shared_ptr<Expr> expr = nullptr)
        : expr_(expr) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetExpr - Get the return expression
     */
    std::shared_ptr<Expr> GetExpr() const { return expr_; }
    
private:
    std::shared_ptr<Expr> expr_;  // The return expression
};

/**
 * DeclStmt - Declaration statement (e.g., int x = 42;)
 */
class DeclStmt : public Stmt {
public:
    /**
     * Constructor
     */
    DeclStmt(std::shared_ptr<Node> decl)
        : decl_(decl) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetDecl - Get the declaration
     */
    std::shared_ptr<Node> GetDecl() const { return decl_; }
    
private:
    std::shared_ptr<Node> decl_;  // The declaration
};

//===----------------------------------------------------------------------===//
// Declarations
//===----------------------------------------------------------------------===//

/**
 * Decl - Base class for all declaration nodes
 */
class Decl : public Node {
public:
    virtual ~Decl() = default;
    
    /**
     * GetName - Get the name of the declaration
     */
    virtual const std::string& GetName() const = 0;
};

/**
 * VarDecl - Variable declaration (e.g., int x = 42;)
 */
class VarDecl : public Decl {
public:
    /**
     * Constructor
     */
    VarDecl(const std::string& name,
            std::shared_ptr<Type> type,
            std::shared_ptr<Expr> init = nullptr)
        : name_(name), type_(type), init_(init) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetName - Get the name of the variable
     */
    const std::string& GetName() const override { return name_; }
    
    /**
     * GetType - Get the type of the variable
     */
    std::shared_ptr<Type> GetType() const { return type_; }
    
    /**
     * GetInit - Get the initializer expression
     */
    std::shared_ptr<Expr> GetInit() const { return init_; }
    
private:
    std::string name_;            // The variable name
    std::shared_ptr<Type> type_;  // The variable type
    std::shared_ptr<Expr> init_;  // The initializer expression
};

/**
 * ParamDecl - Function parameter declaration (e.g., int x in void foo(int x))
 */
class ParamDecl : public Decl {
public:
    /**
     * Constructor
     */
    ParamDecl(const std::string& name, std::shared_ptr<Type> type)
        : name_(name), type_(type) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetName - Get the name of the parameter
     */
    const std::string& GetName() const override { return name_; }
    
    /**
     * GetType - Get the type of the parameter
     */
    std::shared_ptr<Type> GetType() const { return type_; }
    
private:
    std::string name_;            // The parameter name
    std::shared_ptr<Type> type_;  // The parameter type
};

/**
 * FuncDecl - Function declaration (e.g., void foo(int x) { ... })
 */
class FuncDecl : public Decl {
public:
    /**
     * Constructor
     */
    FuncDecl(const std::string& name,
             std::shared_ptr<Type> type,
             std::vector<std::shared_ptr<ParamDecl>> params,
             std::shared_ptr<Stmt> body = nullptr)
        : name_(name), type_(type), params_(params), body_(body) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetName - Get the name of the function
     */
    const std::string& GetName() const override { return name_; }
    
    /**
     * GetType - Get the function type
     */
    std::shared_ptr<Type> GetType() const { return type_; }
    
    /**
     * GetParams - Get the function parameters
     */
    const std::vector<std::shared_ptr<ParamDecl>>& GetParams() const { return params_; }
    
    /**
     * GetBody - Get the function body
     */
    std::shared_ptr<Stmt> GetBody() const { return body_; }
    
private:
    std::string name_;                                // The function name
    std::shared_ptr<Type> type_;                      // The function type
    std::vector<std::shared_ptr<ParamDecl>> params_;  // The function parameters
    std::shared_ptr<Stmt> body_;                      // The function body
};

/**
 * MethodDecl - Objective-C style method declaration (e.g., - (void)foo:(int)x { ... })
 */
class MethodDecl : public Decl {
public:
    /**
     * Constructor
     */
    MethodDecl(const std::string& name,
               std::shared_ptr<Type> type,
               std::shared_ptr<Type> receiver_type,
               std::vector<std::shared_ptr<ParamDecl>> params,
               std::shared_ptr<Stmt> body = nullptr)
        : name_(name), type_(type), receiver_type_(receiver_type), params_(params), body_(body) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetName - Get the name of the method
     */
    const std::string& GetName() const override { return name_; }
    
    /**
     * GetType - Get the method type
     */
    std::shared_ptr<Type> GetType() const { return type_; }
    
    /**
     * GetReceiverType - Get the receiver type
     */
    std::shared_ptr<Type> GetReceiverType() const { return receiver_type_; }
    
    /**
     * GetParams - Get the method parameters
     */
    const std::vector<std::shared_ptr<ParamDecl>>& GetParams() const { return params_; }
    
    /**
     * GetBody - Get the method body
     */
    std::shared_ptr<Stmt> GetBody() const { return body_; }
    
private:
    std::string name_;                                // The method name
    std::shared_ptr<Type> type_;                      // The method type
    std::shared_ptr<Type> receiver_type_;             // The receiver type
    std::vector<std::shared_ptr<ParamDecl>> params_;  // The method parameters
    std::shared_ptr<Stmt> body_;                      // The method body
};

/**
 * StructDecl - Struct declaration (e.g., struct Point { int x; int y; };)
 */
class StructDecl : public Decl {
public:
    /**
     * Constructor
     */
    StructDecl(const std::string& name,
               std::vector<std::shared_ptr<VarDecl>> fields)
        : name_(name), fields_(fields) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetName - Get the name of the struct
     */
    const std::string& GetName() const override { return name_; }
    
    /**
     * GetFields - Get the struct fields
     */
    const std::vector<std::shared_ptr<VarDecl>>& GetFields() const { return fields_; }
    
private:
    std::string name_;                              // The struct name
    std::vector<std::shared_ptr<VarDecl>> fields_;  // The struct fields
};

/**
 * EnumDecl - Enum declaration (e.g., enum Color { RED, GREEN, BLUE };)
 */
class EnumDecl : public Decl {
public:
    /**
     * Constructor
     */
    EnumDecl(const std::string& name,
             std::shared_ptr<Type> base_type,
             std::vector<std::pair<std::string, int64_t>> values)
        : name_(name), base_type_(base_type), values_(values) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetName - Get the name of the enum
     */
    const std::string& GetName() const override { return name_; }
    
    /**
     * GetBaseType - Get the base type of the enum
     */
    std::shared_ptr<Type> GetBaseType() const { return base_type_; }
    
    /**
     * GetValues - Get the enum values
     */
    const std::vector<std::pair<std::string, int64_t>>& GetValues() const { return values_; }
    
private:
    std::string name_;                                  // The enum name
    std::shared_ptr<Type> base_type_;                   // The base type
    std::vector<std::pair<std::string, int64_t>> values_; // The enum values
};

/**
 * CompilationUnit - Top-level AST node for a compilation unit
 */
class CompilationUnit : public Node {
public:
    /**
     * Constructor
     */
    CompilationUnit(std::vector<std::shared_ptr<Decl>> decls)
        : decls_(decls) {}
    
    /**
     * Accept - Accept a visitor to this node
     */
    void Accept(ASTVisitor* visitor) override;
    
    /**
     * GetDecls - Get the declarations
     */
    const std::vector<std::shared_ptr<Decl>>& GetDecls() const { return decls_; }
    
private:
    std::vector<std::shared_ptr<Decl>> decls_;  // The declarations
};

//===----------------------------------------------------------------------===//
// Visitor
//===----------------------------------------------------------------------===//

/**
 * ASTVisitor - Visitor interface for AST nodes
 */
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    // Expressions
    virtual void VisitBinaryExpr(BinaryExpr* expr) = 0;
    virtual void VisitUnaryExpr(UnaryExpr* expr) = 0;
    virtual void VisitLiteralExpr(LiteralExpr* expr) = 0;
    virtual void VisitVarExpr(VarExpr* expr) = 0;
    virtual void VisitAssignExpr(AssignExpr* expr) = 0;
    virtual void VisitCallExpr(CallExpr* expr) = 0;
    virtual void VisitMessageExpr(MessageExpr* expr) = 0;
    virtual void VisitSubscriptExpr(SubscriptExpr* expr) = 0;
    virtual void VisitCastExpr(CastExpr* expr) = 0;
    
    // Statements
    virtual void VisitExprStmt(ExprStmt* stmt) = 0;
    virtual void VisitBlockStmt(BlockStmt* stmt) = 0;
    virtual void VisitIfStmt(IfStmt* stmt) = 0;
    virtual void VisitWhileStmt(WhileStmt* stmt) = 0;
    virtual void VisitForStmt(ForStmt* stmt) = 0;
    virtual void VisitBreakStmt(BreakStmt* stmt) = 0;
    virtual void VisitContinueStmt(ContinueStmt* stmt) = 0;
    virtual void VisitReturnStmt(ReturnStmt* stmt) = 0;
    virtual void VisitDeclStmt(DeclStmt* stmt) = 0;
    
    // Declarations
    virtual void VisitVarDecl(VarDecl* decl) = 0;
    virtual void VisitParamDecl(ParamDecl* decl) = 0;
    virtual void VisitFuncDecl(FuncDecl* decl) = 0;
    virtual void VisitMethodDecl(MethodDecl* decl) = 0;
    virtual void VisitStructDecl(StructDecl* decl) = 0;
    virtual void VisitEnumDecl(EnumDecl* decl) = 0;
    
    // Compilation Unit
    virtual void VisitCompilationUnit(CompilationUnit* unit) = 0;
};

} // namespace dsLang

#endif // DSLANG_AST_H
