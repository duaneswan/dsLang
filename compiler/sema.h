/**
 * sema.h - Semantic Analyzer for dsLang
 * 
 * This file defines the semantic analyzer, which is responsible for
 * type checking, symbol resolution, and other semantic checks on the AST.
 */

#ifndef DS_SEMA_H
#define DS_SEMA_H

#include <memory>
#include "ast.h"

namespace dsLang {

// Forward declarations
class DiagnosticReporter;

/**
 * SemanticAnalyzer - Performs semantic analysis on the AST
 * 
 * This is a simplified version to avoid compilation issues with missing Type implementations
 */
class SemanticAnalyzer : public ASTVisitor {
public:
    /**
     * Constructor
     */
    SemanticAnalyzer(DiagnosticReporter& diag);
    
    /**
     * Analyze - Analyze a compilation unit
     */
    void Analyze(CompilationUnit* unit);
    
    // ASTVisitor implementation - all empty for now
    void VisitCompilationUnit(CompilationUnit* unit) override;
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
    DiagnosticReporter& diag_;
};

/**
 * CreateSemanticAnalyzer - Create a semantic analyzer
 */
std::unique_ptr<SemanticAnalyzer> CreateSemanticAnalyzer(DiagnosticReporter& diag);

} // namespace dsLang

#endif // DS_SEMA_H
