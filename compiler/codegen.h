/**
 * codegen.h - Code Generator for dsLang
 * 
 * This file defines the code generator, which is responsible for
 * converting the AST into LLVM IR and ultimately machine code.
 */

#ifndef DS_CODEGEN_H
#define DS_CODEGEN_H

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <stack>
#include <optional>

// Comment out LLVM headers until we fix include paths
/*
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/MC/TargetRegistry.h>
*/

// Comment out missing headers
// #include <llvm/Support/Host.h>
// #include <llvm/ADT/Optional.h>
#include "ast.h"
#include "type.h"

namespace dsLang {

/**
 * CodeGenError - Exception thrown for code generation errors
 */
class CodeGenError : public std::runtime_error {
public:
    CodeGenError(const std::string& message)
        : std::runtime_error("Code generation error: " + message),
          message_(message) {}
    
    const std::string& GetMessage() const { return message_; }
    
private:
    std::string message_;
};

/**
 * CodeGenerator - Generates LLVM IR from the AST
 */
class CodeGenerator : public ASTVisitor {
public:
    /**
     * Constructor - Initialize the code generator
     */
    CodeGenerator(const std::string& module_name, const std::string& target_triple);
    
    /**
     * Generate - Generate code for a compilation unit
     */
    void Generate(CompilationUnit* unit);
    
    /**
     * EmitIR - Emit LLVM IR to the specified file
     */
    void EmitIR(const std::string& filename);
    
    /**
     * EmitObject - Emit object code to the specified file
     */
    void EmitObject(const std::string& filename);
    
    // ASTVisitor implementation
    void VisitBinaryExpr(BinaryExpr* expr) override;
    void VisitUnaryExpr(UnaryExpr* expr) override;
    void VisitLiteralExpr(LiteralExpr* expr) override;
    void VisitVarExpr(VarExpr* expr) override;
    void VisitAssignExpr(AssignExpr* expr) override;
    void VisitCallExpr(CallExpr* expr) override;
    void VisitMessageExpr(MessageExpr* expr) override;
    void VisitSubscriptExpr(SubscriptExpr* expr) override;
    void VisitCastExpr(CastExpr* expr) override;
    
    void VisitExprStmt(ExprStmt* stmt) override;
    void VisitBlockStmt(BlockStmt* stmt) override;
    void VisitIfStmt(IfStmt* stmt) override;
    void VisitWhileStmt(WhileStmt* stmt) override;
    void VisitForStmt(ForStmt* stmt) override;
    void VisitBreakStmt(BreakStmt* stmt) override;
    void VisitContinueStmt(ContinueStmt* stmt) override;
    void VisitReturnStmt(ReturnStmt* stmt) override;
    void VisitDeclStmt(DeclStmt* stmt) override;
    
    void VisitVarDecl(VarDecl* decl) override;
    void VisitParamDecl(ParamDecl* decl) override;
    void VisitFuncDecl(FuncDecl* decl) override;
    void VisitMethodDecl(MethodDecl* decl) override;
    void VisitStructDecl(StructDecl* decl) override;
    void VisitEnumDecl(EnumDecl* decl) override;
    
private:
    // LLVM context and builder - temporarily commented out until LLVM paths are fixed
    /*
    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;
    std::unique_ptr<llvm::TargetMachine> target_machine_;
    */
    std::string target_triple_;
    
    // Current function being generated
    // llvm::Function* current_function_;
    
    // Symbol table for variable and function lookups
    /*
    std::unordered_map<std::string, llvm::Value*> named_values_;
    std::unordered_map<std::string, llvm::Function*> function_table_;
    std::unordered_map<std::string, llvm::StructType*> struct_types_;
    
    // Value stack for expression evaluation
    std::stack<llvm::Value*> value_stack_;
    */
    
    // Scope management for variable declarations
    struct Scope {
        // std::unordered_map<std::string, llvm::Value*> values;
        std::unordered_map<std::string, void*> values; // Placeholder until LLVM is included
    };
    std::vector<Scope> scopes_;
    
    // Flow control blocks for break and continue statements
    // llvm::BasicBlock* break_target_ = nullptr;
    // llvm::BasicBlock* continue_target_ = nullptr;
    
    // Helper functions
    
    /**
     * DeclareRuntimeFunctions - Declare runtime functions used by the standard library
     */
    void DeclareRuntimeFunctions();
    
    /**
     * BeginScope - Begin a new variable scope
     */
    void BeginScope();
    
    /**
     * EndScope - End the current variable scope
     */
    void EndScope();
    
    /**
     * ConvertType - Convert a dsLang type to an LLVM type
     */
    /* 
    llvm::Type* ConvertType(const std::shared_ptr<Type>& type);
    */
    // Temporary placeholder
    void* ConvertType(const std::shared_ptr<Type>& type);
    
    /**
     * ConvertToBoolean - Convert a value to a boolean
     */
    /*
    llvm::Value* ConvertToBoolean(llvm::Value* value);
    */
    // Temporary placeholder
    void* ConvertToBoolean(void* value);
    
    /**
     * GetLValue - Get the address of an expression for assignment
     */
    /*
    llvm::Value* GetLValue(Expr* expr);
    */
    // Temporary placeholder
    void* GetLValue(Expr* expr);
    
    /**
     * EmitLogicalAnd - Emit code for short-circuit logical AND
     */
    /*
    llvm::Value* EmitLogicalAnd(llvm::Value* lhs, llvm::Value* rhs);
    */
    // Temporary placeholder
    void* EmitLogicalAnd(void* lhs, void* rhs);
    
    /**
     * EmitLogicalOr - Emit code for short-circuit logical OR
     */
    /*
    llvm::Value* EmitLogicalOr(llvm::Value* lhs, llvm::Value* rhs);
    */
    // Temporary placeholder
    void* EmitLogicalOr(void* lhs, void* rhs);
    
    /**
     * EmitPreIncrement - Emit code for pre-increment (++x)
     */
    /*
    llvm::Value* EmitPreIncrement(Expr* expr, llvm::Value* value);
    */
    // Temporary placeholder
    void* EmitPreIncrement(Expr* expr, void* value);
    
    /**
     * EmitPreDecrement - Emit code for pre-decrement (--x)
     */
    /*
    llvm::Value* EmitPreDecrement(Expr* expr, llvm::Value* value);
    */
    // Temporary placeholder
    void* EmitPreDecrement(Expr* expr, void* value);
    
    /**
     * EmitPostIncrement - Emit code for post-increment (x++)
     */
    /*
    llvm::Value* EmitPostIncrement(Expr* expr, llvm::Value* value);
    */
    // Temporary placeholder
    void* EmitPostIncrement(Expr* expr, void* value);
    
    /**
     * EmitPostDecrement - Emit code for post-decrement (x--)
     */
    /*
    llvm::Value* EmitPostDecrement(Expr* expr, llvm::Value* value);
    */
    // Temporary placeholder
    void* EmitPostDecrement(Expr* expr, void* value);
    
    /**
     * IsFloatingPointType - Check if a type is a floating point type
     */
    bool IsFloatingPointType(const std::shared_ptr<Type>& type);
    
    /**
     * IsUnsignedType - Check if a type is an unsigned integer type
     */
    bool IsUnsignedType(const std::shared_ptr<Type>& type);
    
    /**
     * IsIntegerType - Check if a type is an integer type
     */
    bool IsIntegerType(const std::shared_ptr<Type>& type);
    
    /**
     * GetTypeSize - Get the size of a type in bits
     */
    unsigned GetTypeSize(const std::shared_ptr<Type>& type);
};

} // namespace dsLang

#endif // DS_CODEGEN_H
