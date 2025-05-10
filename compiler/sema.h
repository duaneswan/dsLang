/**
 * sema.h - Semantic Analyzer for dsLang
 * 
 * This file defines the semantic analyzer, which is responsible for
 * type checking, symbol resolution, and other semantic checks on the AST.
 */

#ifndef DS_SEMA_H
#define DS_SEMA_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <stack>
#include <stdexcept>
#include "ast.h"

namespace dsLang {

// Forward declarations
class Scope;
class SymbolTable;

/**
 * SemanticError - Exception thrown for semantic errors
 */
class SemanticError : public std::runtime_error {
public:
    SemanticError(const std::string& message, unsigned int line, unsigned int column)
        : std::runtime_error(FormatError(message, line, column)),
          message_(message), line_(line), column_(column) {}
    
    const std::string& GetMessage() const { return message_; }
    unsigned int GetLine() const { return line_; }
    unsigned int GetColumn() const { return column_; }
    
private:
    std::string message_;
    unsigned int line_;
    unsigned int column_;
    
    static std::string FormatError(const std::string& message, unsigned int line, unsigned int column) {
        return "Semantic error at line " + std::to_string(line) + ", column " + 
               std::to_string(column) + ": " + message;
    }
};

/**
 * Symbol - Represents a symbol in the symbol table
 */
class Symbol {
public:
    enum class Kind {
        VARIABLE,
        FUNCTION,
        PARAMETER,
        STRUCT,
        ENUM,
        ENUM_VALUE
    };
    
    Symbol(const std::string& name, Kind kind, std::shared_ptr<Type> type, 
           unsigned int line, unsigned int column)
        : name_(name), kind_(kind), type_(type), line_(line), column_(column) {}
    
    const std::string& GetName() const { return name_; }
    Kind GetKind() const { return kind_; }
    std::shared_ptr<Type> GetType() const { return type_; }
    unsigned int GetLine() const { return line_; }
    unsigned int GetColumn() const { return column_; }
    
private:
    std::string name_;
    Kind kind_;
    std::shared_ptr<Type> type_;
    unsigned int line_;
    unsigned int column_;
};

/**
 * Scope - Represents a scope in the program
 */
class Scope {
public:
    Scope(Scope* parent = nullptr) : parent_(parent) {}
    
    /**
     * Define - Define a symbol in this scope
     */
    void Define(std::shared_ptr<Symbol> symbol) {
        symbols_[symbol->GetName()] = symbol;
    }
    
    /**
     * Resolve - Resolve a symbol by name, checking this scope and parent scopes
     */
    std::shared_ptr<Symbol> Resolve(const std::string& name) {
        auto it = symbols_.find(name);
        if (it != symbols_.end()) {
            return it->second;
        }
        
        if (parent_) {
            return parent_->Resolve(name);
        }
        
        return nullptr;
    }
    
    /**
     * Lookup - Look up a symbol in this scope only
     */
    std::shared_ptr<Symbol> Lookup(const std::string& name) {
        auto it = symbols_.find(name);
        if (it != symbols_.end()) {
            return it->second;
        }
        
        return nullptr;
    }
    
    /**
     * GetParent - Get the parent scope
     */
    Scope* GetParent() const { return parent_; }
    
private:
    Scope* parent_;
    std::unordered_map<std::string, std::shared_ptr<Symbol>> symbols_;
};

/**
 * SymbolTable - Manages the symbol table for the program
 */
class SymbolTable {
public:
    SymbolTable() {
        // Create the global scope
        EnterScope();
        
        // Add built-in types
        DefineBuiltinTypes();
    }
    
    /**
     * EnterScope - Enter a new scope
     */
    void EnterScope() {
        scopes_.push_back(std::make_unique<Scope>(CurrentScope()));
    }
    
    /**
     * ExitScope - Exit the current scope
     */
    void ExitScope() {
        if (!scopes_.empty()) {
            scopes_.pop_back();
        }
    }
    
    /**
     * CurrentScope - Get the current scope
     */
    Scope* CurrentScope() {
        return scopes_.empty() ? nullptr : scopes_.back().get();
    }
    
    /**
     * Define - Define a symbol in the current scope
     */
    void Define(std::shared_ptr<Symbol> symbol) {
        CurrentScope()->Define(symbol);
    }
    
    /**
     * Resolve - Resolve a symbol by name
     */
    std::shared_ptr<Symbol> Resolve(const std::string& name) {
        return CurrentScope()->Resolve(name);
    }
    
    /**
     * GetBuiltinType - Get a built-in type by kind
     */
    std::shared_ptr<Type> GetBuiltinType(Type::Kind kind) {
        return builtin_types_[static_cast<int>(kind)];
    }
    
private:
    std::vector<std::unique_ptr<Scope>> scopes_;
    std::vector<std::shared_ptr<Type>> builtin_types_;
    
    /**
     * DefineBuiltinTypes - Define built-in types
     */
    void DefineBuiltinTypes() {
        builtin_types_.resize(static_cast<int>(Type::Kind::ARRAY) + 1);
        
        // Define basic types
        builtin_types_[static_cast<int>(Type::Kind::VOID)] = std::make_shared<BasicType>(Type::Kind::VOID);
        builtin_types_[static_cast<int>(Type::Kind::BOOL)] = std::make_shared<BasicType>(Type::Kind::BOOL);
        builtin_types_[static_cast<int>(Type::Kind::CHAR)] = std::make_shared<BasicType>(Type::Kind::CHAR);
        builtin_types_[static_cast<int>(Type::Kind::INT)] = std::make_shared<BasicType>(Type::Kind::INT);
        builtin_types_[static_cast<int>(Type::Kind::LONG)] = std::make_shared<BasicType>(Type::Kind::LONG);
        builtin_types_[static_cast<int>(Type::Kind::SHORT)] = std::make_shared<BasicType>(Type::Kind::SHORT);
        builtin_types_[static_cast<int>(Type::Kind::UNSIGNED_INT)] = std::make_shared<BasicType>(Type::Kind::UNSIGNED_INT);
        builtin_types_[static_cast<int>(Type::Kind::UNSIGNED_LONG)] = std::make_shared<BasicType>(Type::Kind::UNSIGNED_LONG);
        builtin_types_[static_cast<int>(Type::Kind::UNSIGNED_CHAR)] = std::make_shared<BasicType>(Type::Kind::UNSIGNED_CHAR);
        builtin_types_[static_cast<int>(Type::Kind::UNSIGNED_SHORT)] = std::make_shared<BasicType>(Type::Kind::UNSIGNED_SHORT);
    }
};

/**
 * SemanticAnalyzer - Performs semantic analysis on the AST
 */
class SemanticAnalyzer : public ASTVisitor {
public:
    SemanticAnalyzer() : current_function_(nullptr) {}
    
    /**
     * Analyze - Analyze a compilation unit
     */
    void Analyze(CompilationUnit* unit) {
        // First pass: collect declarations
        for (const auto& decl : unit->GetDecls()) {
            CollectDeclaration(decl.get());
        }
        
        // Second pass: analyze declarations
        for (const auto& decl : unit->GetDecls()) {
            decl->Accept(this);
        }
    }
    
    /**
     * VisitBinaryExpr - Visit a binary expression node
     */
    void VisitBinaryExpr(BinaryExpr* expr) override {
        expr->GetLeft()->Accept(this);
        expr->GetRight()->Accept(this);
        
        auto left_type = expr->GetLeft()->GetType();
        auto right_type = expr->GetRight()->GetType();
        
        // Ensure both operands have compatible types
        if (!AreTypesCompatible(left_type, right_type)) {
            ThrowError("Incompatible types for binary operator", expr->GetLine(), expr->GetColumn());
        }
        
        // Set the result type based on the operator
        switch (expr->GetOp()) {
            // Arithmetic operators
            case BinaryExpr::Op::ADD:
            case BinaryExpr::Op::SUB:
            case BinaryExpr::Op::MUL:
            case BinaryExpr::Op::DIV:
            case BinaryExpr::Op::MOD:
                expr->SetType(GetCommonType(left_type, right_type));
                break;
                
            // Bitwise operators
            case BinaryExpr::Op::BIT_AND:
            case BinaryExpr::Op::BIT_OR:
            case BinaryExpr::Op::BIT_XOR:
            case BinaryExpr::Op::SHIFT_LEFT:
            case BinaryExpr::Op::SHIFT_RIGHT:
                expr->SetType(GetCommonType(left_type, right_type));
                break;
                
            // Comparison operators
            case BinaryExpr::Op::EQUAL:
            case BinaryExpr::Op::NOT_EQUAL:
            case BinaryExpr::Op::LESS:
            case BinaryExpr::Op::GREATER:
            case BinaryExpr::Op::LESS_EQUAL:
            case BinaryExpr::Op::GREATER_EQUAL:
                // Comparison operators return bool
                expr->SetType(symbols_.GetBuiltinType(Type::Kind::BOOL));
                break;
                
            // Logical operators
            case BinaryExpr::Op::LOGICAL_AND:
            case BinaryExpr::Op::LOGICAL_OR:
                // Logical operators operate on and return bool
                expr->SetType(symbols_.GetBuiltinType(Type::Kind::BOOL));
                break;
        }
    }
    
    /**
     * VisitUnaryExpr - Visit a unary expression node
     */
    void VisitUnaryExpr(UnaryExpr* expr) override {
        expr->GetOperand()->Accept(this);
        
        auto operand_type = expr->GetOperand()->GetType();
        
        // Set the result type based on the operator
        switch (expr->GetOp()) {
            case UnaryExpr::Op::NEGATE:
                // Negation works on numeric types
                if (!IsNumericType(operand_type)) {
                    ThrowError("Cannot negate non-numeric type", expr->GetLine(), expr->GetColumn());
                }
                expr->SetType(operand_type);
                break;
                
            case UnaryExpr::Op::NOT:
                // Logical not returns bool
                expr->SetType(symbols_.GetBuiltinType(Type::Kind::BOOL));
                break;
                
            case UnaryExpr::Op::BIT_NOT:
                // Bitwise not works on integer types
                if (!IsIntegerType(operand_type)) {
                    ThrowError("Bitwise not requires integer type", expr->GetLine(), expr->GetColumn());
                }
                expr->SetType(operand_type);
                break;
                
            case UnaryExpr::Op::PRE_INC:
            case UnaryExpr::Op::PRE_DEC:
            case UnaryExpr::Op::POST_INC:
            case UnaryExpr::Op::POST_DEC:
                // Increment/decrement require numeric types
                if (!IsNumericType(operand_type)) {
                    ThrowError("Increment/decrement requires numeric type", expr->GetLine(), expr->GetColumn());
                }
                expr->SetType(operand_type);
                break;
                
            case UnaryExpr::Op::DEREF:
                // Dereference requires pointer type
                if (operand_type->GetKind() != Type::Kind::POINTER) {
                    ThrowError("Cannot dereference non-pointer type", expr->GetLine(), expr->GetColumn());
                }
                expr->SetType(static_cast<PointerType*>(operand_type.get())->GetPointeeType());
                break;
                
            case UnaryExpr::Op::ADDR_OF:
                // Address-of returns pointer to operand type
                expr->SetType(std::make_shared<PointerType>(operand_type));
                break;
        }
    }
    
    /**
     * VisitLiteralExpr - Visit a literal expression node
     */
    void VisitLiteralExpr(LiteralExpr* expr) override {
        // Set the type based on the literal kind
        switch (expr->GetKind()) {
            case LiteralExpr::Kind::INTEGER:
                expr->SetType(symbols_.GetBuiltinType(Type::Kind::INT));
                break;
                
            case LiteralExpr::Kind::CHARACTER:
                expr->SetType(symbols_.GetBuiltinType(Type::Kind::CHAR));
                break;
                
            case LiteralExpr::Kind::STRING:
                // String literals are pointers to char
                expr->SetType(std::make_shared<PointerType>(symbols_.GetBuiltinType(Type::Kind::CHAR)));
                break;
                
            case LiteralExpr::Kind::BOOLEAN:
                expr->SetType(symbols_.GetBuiltinType(Type::Kind::BOOL));
                break;
                
            case LiteralExpr::Kind::NULL_VALUE:
                // Null is a special case, it can be assigned to any pointer
                expr->SetType(std::make_shared<PointerType>(symbols_.GetBuiltinType(Type::Kind::VOID)));
                break;
        }
    }
    
    /**
     * VisitVarExpr - Visit a variable expression node
     */
    void VisitVarExpr(VarExpr* expr) override {
        const std::string& name = expr->GetName();
        auto symbol = symbols_.Resolve(name);
        
        if (!symbol) {
            ThrowError("Undefined variable: " + name, expr->GetLine(), expr->GetColumn());
        }
        
        expr->SetType(symbol->GetType());
    }
    
    /**
     * VisitAssignExpr - Visit an assignment expression node
     */
    void VisitAssignExpr(AssignExpr* expr) override {
        expr->GetTarget()->Accept(this);
        expr->GetValue()->Accept(this);
        
        auto target_type = expr->GetTarget()->GetType();
        auto value_type = expr->GetValue()->GetType();
        
        // Ensure the target is a valid l-value
        // TODO: Implement more rigorous l-value checking
        
        // Ensure the value type is compatible with the target type
        if (!AreTypesCompatible(target_type, value_type)) {
            ThrowError("Incompatible types in assignment", expr->GetLine(), expr->GetColumn());
        }
        
        // The result type of the assignment is the target type
        expr->SetType(target_type);
    }
    
    /**
     * VisitCallExpr - Visit a function call expression node
     */
    void VisitCallExpr(CallExpr* expr) override {
        expr->GetCallee()->Accept(this);
        
        // Ensure the callee is a function pointer
        auto callee_type = expr->GetCallee()->GetType();
        if (callee_type->GetKind() != Type::Kind::POINTER) {
            ThrowError("Called object is not a function", expr->GetLine(), expr->GetColumn());
        }
        
        // TODO: Check function pointer signature against arguments
        
        // For simplicity, assume the return type is int
        // This is a placeholder and should be replaced with proper function type handling
        expr->SetType(symbols_.GetBuiltinType(Type::Kind::INT));
        
        // Visit and type check the arguments
        for (const auto& arg : expr->GetArgs()) {
            arg->Accept(this);
        }
    }
    
    /**
     * VisitMessageExpr - Visit a message expression node
     */
    void VisitMessageExpr(MessageExpr* expr) override {
        expr->GetReceiver()->Accept(this);
        
        // For now, we'll treat message expressions like function calls
        // A more sophisticated implementation would handle method lookup
        
        // Visit arguments if any
        if (expr->HasArgs()) {
            for (const auto& arg : expr->GetSelectorArgs()) {
                arg.arg->Accept(this);
            }
        }
        
        // For simplicity, assume the result type is int
        // This is a placeholder and should be replaced with proper method type handling
        expr->SetType(symbols_.GetBuiltinType(Type::Kind::INT));
    }
    
    /**
     * VisitSubscriptExpr - Visit a subscript expression node
     */
    void VisitSubscriptExpr(SubscriptExpr* expr) override {
        expr->GetArray()->Accept(this);
        expr->GetIndex()->Accept(this);
        
        auto array_type = expr->GetArray()->GetType();
        auto index_type = expr->GetIndex()->GetType();
        
        // Check that the index is an integer type
        if (!IsIntegerType(index_type)) {
            ThrowError("Array index must be an integer", expr->GetLine(), expr->GetColumn());
        }
        
        // Check that the array is an array or pointer type
        if (array_type->GetKind() == Type::Kind::ARRAY) {
            expr->SetType(static_cast<ArrayType*>(array_type.get())->GetElementType());
        } else if (array_type->GetKind() == Type::Kind::POINTER) {
            expr->SetType(static_cast<PointerType*>(array_type.get())->GetPointeeType());
        } else {
            ThrowError("Subscript requires array or pointer type", expr->GetLine(), expr->GetColumn());
        }
    }
    
    /**
     * VisitCastExpr - Visit a cast expression node
     */
    void VisitCastExpr(CastExpr* expr) override {
        expr->GetOperand()->Accept(this);
        
        // Set the type directly to the target type
        expr->SetType(expr->GetTargetType());
    }
    
    /**
     * VisitExprStmt - Visit an expression statement node
     */
    void VisitExprStmt(ExprStmt* stmt) override {
        stmt->GetExpr()->Accept(this);
    }
    
    /**
     * VisitBlockStmt - Visit a block statement node
     */
    void VisitBlockStmt(BlockStmt* stmt) override {
        // Enter a new scope for the block
        symbols_.EnterScope();
        
        // Visit all statements in the block
        for (const auto& s : stmt->GetStmts()) {
            s->Accept(this);
        }
        
        // Exit the block scope
        symbols_.ExitScope();
    }
    
    /**
     * VisitIfStmt - Visit an if statement node
     */
    void VisitIfStmt(IfStmt* stmt) override {
        stmt->GetCondition()->Accept(this);
        
        // Ensure the condition has a boolean-compatible type
        auto condition_type = stmt->GetCondition()->GetType();
        if (!IsScalarType(condition_type)) {
            ThrowError("If condition must be a scalar type", stmt->GetLine(), stmt->GetColumn());
        }
        
        // Visit the then branch
        stmt->GetThenBranch()->Accept(this);
        
        // Visit the else branch if present
        if (stmt->GetElseBranch()) {
            stmt->GetElseBranch()->Accept(this);
        }
    }
    
    /**
     * VisitWhileStmt - Visit a while statement node
     */
    void VisitWhileStmt(WhileStmt* stmt) override {
        stmt->GetCondition()->Accept(this);
        
        // Ensure the condition has a boolean-compatible type
        auto condition_type = stmt->GetCondition()->GetType();
        if (!IsScalarType(condition_type)) {
            ThrowError("While condition must be a scalar type", stmt->GetLine(), stmt->GetColumn());
        }
        
        // Track that we're in a loop for break/continue validation
        in_loop_++;
        
        // Visit the body
        stmt->GetBody()->Accept(this);
        
        // Leave the loop context
        in_loop_--;
    }
    
    /**
     * VisitForStmt - Visit a for statement node
     */
    void VisitForStmt(ForStmt* stmt) override {
        // Enter a new scope for the for loop
        symbols_.EnterScope();
        
        // Visit the initializer if present
        if (stmt->GetInit()) {
            stmt->GetInit()->Accept(this);
        }
        
        // Visit the condition if present
        if (stmt->GetCondition()) {
            stmt->GetCondition()->Accept(this);
            
            // Ensure the condition has a boolean-compatible type
            auto condition_type = stmt->GetCondition()->GetType();
            if (!IsScalarType(condition_type)) {
                ThrowError("For condition must be a scalar type", stmt->GetLine(), stmt->GetColumn());
            }
        }
        
        // Visit the increment if present
        if (stmt->GetIncrement()) {
            stmt->GetIncrement()->Accept(this);
        }
        
        // Track that we're in a loop for break/continue validation
        in_loop_++;
        
        // Visit the body
        stmt->GetBody()->Accept(this);
        
        // Leave the loop context
        in_loop_--;
        
        // Exit the for loop scope
        symbols_.ExitScope();
    }
    
    /**
     * VisitReturnStmt - Visit a return statement node
     */
    void VisitReturnStmt(ReturnStmt* stmt) override {
        // Ensure we're in a function
        if (!current_function_) {
            ThrowError("Return statement outside function", stmt->GetLine(), stmt->GetColumn());
        }
        
        // Check the return value if present
        if (stmt->GetValue()) {
            stmt->GetValue()->Accept(this);
            
            auto return_type = current_function_->GetReturnType();
            auto value_type = stmt->GetValue()->GetType();
            
            // Ensure the return value has a compatible type
            if (!AreTypesCompatible(return_type, value_type)) {
                ThrowError("Incompatible return type", stmt->GetLine(), stmt->GetColumn());
            }
        } else {
            // Empty return must be in a void function
            auto return_type = current_function_->GetReturnType();
            if (return_type->GetKind() != Type::Kind::VOID) {
                ThrowError("Non-void function must return a value", stmt->GetLine(), stmt->GetColumn());
            }
        }
    }
    
    /**
     * VisitBreakStmt - Visit a break statement node
     */
    void VisitBreakStmt(BreakStmt* stmt) override {
        // Ensure we're in a loop
        if (in_loop_ <= 0) {
            ThrowError("Break statement outside loop", stmt->GetLine(), stmt->GetColumn());
        }
    }
    
    /**
     * VisitContinueStmt - Visit a continue statement node
     */
    void VisitContinueStmt(ContinueStmt* stmt) override {
        // Ensure we're in a loop
        if (in_loop_ <= 0) {
            ThrowError("Continue statement outside loop", stmt->GetLine(), stmt->GetColumn());
        }
    }
    
    /**
     * VisitVarDecl - Visit a variable declaration node
     */
    void VisitVarDecl(VarDecl* decl) override {
        // Variable should already be in the symbol table from the first pass
        
        // Check the initializer if present
        if (decl->GetInitializer()) {
            decl->GetInitializer()->Accept(this);
            
            auto var_type = decl->GetType();
            auto init_type = decl->GetInitializer()->GetType();
            
            // Ensure the initializer has a compatible type
            if (!AreTypesCompatible(var_type, init_type)) {
                ThrowError("Incompatible initializer type", decl->GetLine(), decl->GetColumn());
            }
        }
    }
    
    /**
     * VisitFuncDecl - Visit a function declaration node
     */
    void VisitFuncDecl(FuncDecl* decl) override {
        // Function should already be in the symbol table from the first pass
        
        // Save the current function and set the new one
        auto old_function = current_function_;
        current_function_ = decl;
        
        // Enter a new scope for the function body
        symbols_.EnterScope();
        
        // Add parameters to the function scope
        for (const auto& param : decl->GetParams()) {
            auto param_symbol = std::make_shared<Symbol>(
                param->GetName(), Symbol::Kind::PARAMETER, param->GetType(),
                param->GetLine(), param->GetColumn());
            symbols_.Define(param_symbol);
        }
        
        // Visit the function body if present
        if (decl->GetBody()) {
            decl->GetBody()->Accept(this);
        }
        
        // Exit the function scope
        symbols_.ExitScope();
        
        // Restore the previous function
        current_function_ = old_function;
    }
    
    /**
     * VisitStructDecl - Visit a struct declaration node
     */
    void VisitStructDecl(StructDecl* decl) override {
        // Struct should already be in the symbol table from the first pass
        
        // Visit the struct fields
        for (const auto& field : decl->GetFields()) {
            field->Accept(this);
        }
    }
    
    /**
     * VisitEnumDecl - Visit an enum declaration node
     */
    void VisitEnumDecl(EnumDecl* decl) override {
        // Enum should already be in the symbol table from the first pass
        
        // Visit the enum values
        for (const auto& value : decl->GetValues()) {
            if (value->GetValue()) {
                value->GetValue()->Accept(this);
                
                // Ensure the value has an integer type
                auto value_type = value->GetValue()->GetType();
                if (!IsIntegerType(value_type)) {
                    ThrowError("Enum value must be an integer", value->GetLine(), value->GetColumn());
                }
            }
        }
    }
    
    /**
     * VisitParamDecl - Visit a parameter declaration node
     */
    void VisitParamDecl(ParamDecl* decl) override {
        // Parameters are handled in VisitFuncDecl
    }
    
private:
    SymbolTable symbols_;
    FuncDecl* current_function_;
    int in_loop_ = 0;
    
    /**
     * ThrowError - Throw a semantic error
     */
    void ThrowError(const std::string& message, unsigned int line, unsigned int column) {
        throw SemanticError(message, line, column);
    }
    
    /**
     * CollectDeclaration - Collect a declaration in the first pass
     */
    void CollectDeclaration(Decl* decl) {
        if (auto var_decl = dynamic_cast<VarDecl*>(decl)) {
            // Add variable to the symbol table
            auto symbol = std::make_shared<Symbol>(
                var_decl->GetName(), Symbol::Kind::VARIABLE, var_decl->GetType(),
                var_decl->GetLine(), var_decl->GetColumn());
            symbols_.Define(symbol);
        } else if (auto func_decl = dynamic_cast<FuncDecl*>(decl)) {
            // Add function to the symbol table
            auto symbol = std::make_shared<Symbol>(
                func_decl->GetName(), Symbol::Kind::FUNCTION, func_decl->GetReturnType(),
                func_decl->GetLine(), func_decl->GetColumn());
            symbols_.Define(symbol);
        } else if (auto struct_decl = dynamic_cast<StructDecl*>(decl)) {
            // Add struct to the symbol table
            auto struct_type = std::make_shared<StructType>(struct_decl->GetName());
            auto symbol = std::make_shared<Symbol>(
                struct_decl->GetName(), Symbol::Kind::STRUCT, struct_type,
                struct_decl->GetLine(), struct_decl->GetColumn());
            symbols_.Define(symbol);
        } else if (auto enum_decl = dynamic_cast<EnumDecl*>(decl)) {
            // Add enum to the symbol table
            auto enum_type = std::make_shared<EnumType>(enum_decl->GetName());
            auto symbol = std::make_shared<Symbol>(
                enum_decl->GetName(), Symbol::Kind::ENUM, enum_type,
                enum_decl->GetLine(), enum_decl->GetColumn());
            symbols_.Define(symbol);
            
            // Add enum values to the symbol table
            for (const auto& value : enum_decl->GetValues()) {
                auto value_symbol = std::make_shared<Symbol>(
                    value->GetName(), Symbol::Kind::ENUM_VALUE, enum_type,
                    value->GetLine(), value->GetColumn());
                symbols_.Define(value_symbol);
            }
        }
    }
    
    /**
     * Type utility functions
     */
    
    bool IsIntegerType(std::shared_ptr<Type> type) {
        switch (type->GetKind()) {
            case Type::Kind::CHAR:
            case Type::Kind::INT:
            case Type::Kind::LONG:
            case Type::Kind::SHORT:
            case Type::Kind::UNSIGNED_CHAR:
            case Type::Kind::UNSIGNED_INT:
            case Type::Kind::UNSIGNED_LONG:
            case Type::Kind::UNSIGNED_SHORT:
                return true;
            default:
                return false;
        }
    }
    
    bool IsNumericType(std::shared_ptr<Type> type) {
        return IsIntegerType(type); // TODO: Add floating-point types
    }
    
    bool IsScalarType(std::shared_ptr<Type> type) {
        switch (type->GetKind()) {
            case Type::Kind::BOOL:
            case Type::Kind::CHAR:
            case Type::Kind::INT:
            case Type::Kind::LONG:
            case Type::Kind::SHORT:
            case Type::Kind::UNSIGNED_CHAR:
            case Type::Kind::UNSIGNED_INT:
            case Type::Kind::UNSIGNED_LONG:
            case Type::Kind::UNSIGNED_SHORT:
            case Type::Kind::POINTER:
                return true;
            default:
                return false;
        }
    }
    
    bool AreTypesCompatible(std::shared_ptr<Type> t1, std::shared_ptr<Type> t2) {
        // Same type
        if (t1->GetKind() == t2->GetKind()) {
            return true;
        }
        
        // Integer types are compatible with each other
        if (IsIntegerType(t1) && IsIntegerType(t2)) {
            return true;
        }
        
        // Null can be assigned to any pointer
        if (t1->GetKind() == Type::Kind::POINTER &&
            t2->GetKind() == Type::Kind::POINTER) {
            auto t2_pointee = static_cast<PointerType*>(t2.get())->GetPointeeType();
            if (t2_pointee->GetKind() == Type::Kind::VOID) {
                return true;
            }
        }
        
        // Void pointers can be assigned from any pointer
        if (t1->GetKind() == Type::Kind::POINTER &&
            t2->GetKind() == Type::Kind::POINTER) {
            auto t1_pointee = static_cast<PointerType*>(t1.get())->GetPointeeType();
            if (t1_pointee->GetKind() == Type::Kind::VOID) {
                return true;
            }
        }
        
        return false;
    }
    
    std::shared_ptr<Type> GetCommonType(std::shared_ptr<Type> t1, std::shared_ptr<Type> t2) {
        // If types are the same, return either
        if (t1->GetKind() == t2->GetKind()) {
            return t1;
        }
        
        // For mixed integer types, promote to the larger type
        // This is a simplified promotion strategy
        if (IsIntegerType(t1) && IsIntegerType(t2)) {
            if (t1->GetKind() == Type::Kind::LONG || t2->GetKind() == Type::Kind::LONG) {
                return symbols_.GetBuiltinType(Type::Kind::LONG);
            } else if (t1->GetKind() == Type::Kind::INT || t2->GetKind() == Type::Kind::INT) {
                return symbols_.GetBuiltinType(Type::Kind::INT);
            } else if (t1->GetKind() == Type::Kind::SHORT || t2->GetKind() == Type::Kind::SHORT) {
                return symbols_.GetBuiltinType(Type::Kind::SHORT);
            } else {
                return symbols_.GetBuiltinType(Type::Kind::CHAR);
            }
        }
        
        // Default: just return t1
        return t1;
    }
};

} // namespace dsLang

#endif // DS_SEMA_H
