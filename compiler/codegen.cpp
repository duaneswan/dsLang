/**
 * codegen.cpp - Code Generator Implementation for dsLang
 * 
 * This file implements the code generator for dsLang, converting
 * the AST into LLVM IR and ultimately machine code.
 */

#include "codegen.h"
#include <sstream>
#include <iostream>

namespace dsLang {

/**
 * Constructor - Initialize the code generator
 */
CodeGenerator::CodeGenerator(const std::string& module_name, const std::string& target_triple)
    : context_(std::make_unique<llvm::LLVMContext>()),
      module_(std::make_unique<llvm::Module>(module_name, *context_)),
      builder_(std::make_unique<llvm::IRBuilder<>>(*context_)),
      target_triple_(target_triple),
      current_function_(nullptr) {
    
    // Initialize LLVM targets
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    
    // Set the target triple
    module_->setTargetTriple(target_triple_);
    
    // Initialize the target machine
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(target_triple_, error);
    
    if (!target) {
        std::cerr << "Failed to lookup target: " << error << std::endl;
        return;
    }
    
    auto cpu = "generic";
    auto features = "";
    
    llvm::TargetOptions opt;
    // Use std::nullopt instead of std::optional<llvm::Reloc::Model>()
    llvm::Optional<llvm::Reloc::Model> rm;
    target_machine_ = std::unique_ptr<llvm::TargetMachine>(
        target->createTargetMachine(target_triple_, cpu, features, opt, rm));
    
    module_->setDataLayout(target_machine_->createDataLayout());

    // Create the initial scope
    BeginScope();
}

/**
 * Generate - Generate code for a compilation unit
 */
void CodeGenerator::Generate(CompilationUnit* unit) {
    // Add runtime functions and structs
    DeclareRuntimeFunctions();
    
    // Process all declarations
    for (const auto& decl : unit->GetDecls()) {
        decl->Accept(this);
    }
    
    // Verify the module
    std::string error;
    llvm::raw_string_ostream error_stream(error);
    if (llvm::verifyModule(*module_, &error_stream)) {
        std::cerr << "Module verification failed: " << error << std::endl;
        return;
    }
}

/**
 * EmitIR - Emit LLVM IR to the specified file
 */
void CodeGenerator::EmitIR(const std::string& filename) {
    std::error_code ec;
    llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);
    
    if (ec) {
        std::cerr << "Could not open file: " << ec.message() << std::endl;
        return;
    }
    
    module_->print(dest, nullptr);
}

/**
 * EmitObject - Emit object code to the specified file
 */
void CodeGenerator::EmitObject(const std::string& filename) {
    std::error_code ec;
    llvm::raw_fd_ostream dest(filename, ec, llvm::sys::fs::OF_None);
    
    if (ec) {
        std::cerr << "Could not open file: " << ec.message() << std::endl;
        return;
    }
    
    llvm::legacy::PassManager pass;
    auto file_type = llvm::CodeGenFileType::ObjectFile;
    
    if (target_machine_->addPassesToEmitFile(pass, dest, nullptr, file_type)) {
        std::cerr << "Target machine can't emit a file of this type" << std::endl;
        return;
    }
    
    pass.run(*module_);
    dest.flush();
}

//===----------------------------------------------------------------------===//
// Helper methods
//===----------------------------------------------------------------------===//

/**
 * DeclareRuntimeFunctions - Declare runtime functions used by the standard library
 */
void CodeGenerator::DeclareRuntimeFunctions() {
    // void* malloc(size_t size)
    {
        llvm::FunctionType* malloc_type = llvm::FunctionType::get(
            llvm::PointerType::get(*context_, 0),
            {llvm::Type::getInt64Ty(*context_)},
            false);
        module_->getOrInsertFunction("malloc", malloc_type);
    }
    
    // void free(void* ptr)
    {
        llvm::FunctionType* free_type = llvm::FunctionType::get(
            llvm::Type::getVoidTy(*context_),
            {llvm::PointerType::get(*context_, 0)},
            false);
        module_->getOrInsertFunction("free", free_type);
    }
    
    // void memcpy(void* dest, void* src, size_t n)
    {
        llvm::FunctionType* memcpy_type = llvm::FunctionType::get(
            llvm::Type::getVoidTy(*context_),
            {
                llvm::PointerType::get(*context_, 0),
                llvm::PointerType::get(*context_, 0),
                llvm::Type::getInt64Ty(*context_)
            },
            false);
        module_->getOrInsertFunction("memcpy", memcpy_type);
    }
    
    // void memset(void* s, int c, size_t n)
    {
        llvm::FunctionType* memset_type = llvm::FunctionType::get(
            llvm::Type::getVoidTy(*context_),
            {
                llvm::PointerType::get(*context_, 0),
                llvm::Type::getInt32Ty(*context_),
                llvm::Type::getInt64Ty(*context_)
            },
            false);
        module_->getOrInsertFunction("memset", memset_type);
    }
    
    // void putchar(char c)
    {
        llvm::FunctionType* putchar_type = llvm::FunctionType::get(
            llvm::Type::getVoidTy(*context_),
            {llvm::Type::getInt8Ty(*context_)},
            false);
        module_->getOrInsertFunction("putchar", putchar_type);
    }
    
    // void puts(const char* s)
    {
        llvm::FunctionType* puts_type = llvm::FunctionType::get(
            llvm::Type::getVoidTy(*context_),
            {llvm::PointerType::get(llvm::Type::getInt8Ty(*context_), 0)},
            false);
        module_->getOrInsertFunction("puts", puts_type);
    }
    
    // char* strcpy(char* dest, const char* src)
    {
        llvm::FunctionType* strcpy_type = llvm::FunctionType::get(
            llvm::PointerType::get(llvm::Type::getInt8Ty(*context_), 0),
            {
                llvm::PointerType::get(llvm::Type::getInt8Ty(*context_), 0),
                llvm::PointerType::get(llvm::Type::getInt8Ty(*context_), 0)
            },
            false);
        module_->getOrInsertFunction("strcpy", strcpy_type);
    }
    
    // void outb(uint16_t port, uint8_t val)
    {
        llvm::FunctionType* outb_type = llvm::FunctionType::get(
            llvm::Type::getVoidTy(*context_),
            {
                llvm::Type::getInt16Ty(*context_),
                llvm::Type::getInt8Ty(*context_)
            },
            false);
        module_->getOrInsertFunction("outb", outb_type);
    }
    
    // uint8_t inb(uint16_t port)
    {
        llvm::FunctionType* inb_type = llvm::FunctionType::get(
            llvm::Type::getInt8Ty(*context_),
            {llvm::Type::getInt16Ty(*context_)},
            false);
        module_->getOrInsertFunction("inb", inb_type);
    }
}

/**
 * BeginScope - Begin a new variable scope
 */
void CodeGenerator::BeginScope() {
    scopes_.push_back(Scope());
}

/**
 * EndScope - End the current variable scope
 */
void CodeGenerator::EndScope() {
    // Remove all variables in the current scope from named_values
    for (const auto& entry : scopes_.back().values) {
        named_values_.erase(entry.first);
    }
    
    // Pop the scope
    scopes_.pop_back();
}

/**
 * ConvertType - Convert a dsLang type to an LLVM type
 */
llvm::Type* CodeGenerator::ConvertType(const std::shared_ptr<Type>& type) {
    switch (type->GetKind()) {
        case Type::Kind::VOID:
            return llvm::Type::getVoidTy(*context_);
            
        case Type::Kind::BOOL:
            return llvm::Type::getInt1Ty(*context_);
            
        case Type::Kind::CHAR:
            return llvm::Type::getInt8Ty(*context_);
            
        case Type::Kind::SHORT:
            return llvm::Type::getInt16Ty(*context_);
            
        case Type::Kind::INT:
            return llvm::Type::getInt32Ty(*context_);
            
        case Type::Kind::LONG:
            return llvm::Type::getInt64Ty(*context_);
            
        case Type::Kind::FLOAT:
            return llvm::Type::getFloatTy(*context_);
            
        case Type::Kind::DOUBLE:
            return llvm::Type::getDoubleTy(*context_);
            
        case Type::Kind::POINTER: {
            auto ptr_type = std::static_pointer_cast<PointerType>(type);
            return llvm::PointerType::get(ConvertType(ptr_type->GetPointeeType()), 0);
        }
            
        case Type::Kind::ARRAY: {
            auto array_type = std::static_pointer_cast<ArrayType>(type);
            return llvm::ArrayType::get(
                ConvertType(array_type->GetElementType()),
                array_type->GetSize());
        }
            
        case Type::Kind::FUNCTION: {
            auto func_type = std::static_pointer_cast<FunctionType>(type);
            
            std::vector<llvm::Type*> param_types;
            for (const auto& param_type : func_type->GetParamTypes()) {
                param_types.push_back(ConvertType(param_type));
            }
            
            return llvm::PointerType::get(
                llvm::FunctionType::get(
                    ConvertType(func_type->GetReturnType()),
                    param_types,
                    func_type->IsVariadic()),
                0);
        }
            
        case Type::Kind::STRUCT: {
            auto struct_type = std::static_pointer_cast<StructType>(type);
            auto name = struct_type->GetName();
            
            // If the struct type is already defined, return it
            auto it = struct_types_.find(name);
            if (it != struct_types_.end()) {
                return it->second;
            }
            
            // Otherwise, create a new struct type
            llvm::StructType* llvm_struct_type = llvm::StructType::create(
                *context_,
                name);
            struct_types_[name] = llvm_struct_type;
            
            // Set the body of the struct type
            std::vector<llvm::Type*> field_types;
            for (const auto& field_pair : struct_type->GetFields()) {
                field_types.push_back(ConvertType(field_pair.second));
            }
            
            llvm_struct_type->setBody(field_types);
            
            return llvm_struct_type;
        }
            
        default:
            std::cerr << "Unsupported type kind: " << std::to_string(static_cast<int>(type->GetKind())) << std::endl;
            return nullptr;
    }
}

/**
 * ConvertToBoolean - Convert a value to a boolean
 */
llvm::Value* CodeGenerator::ConvertToBoolean(llvm::Value* value) {
    // If the value is already a boolean (i1), return it as is
    if (value->getType()->isIntegerTy(1)) {
        return value;
    }
    
    // If the value is an integer, compare it with zero
    if (value->getType()->isIntegerTy()) {
        return builder_->CreateICmpNE(
            value,
            llvm::ConstantInt::get(value->getType(), 0));
    }
    
    // If the value is a floating point, compare it with zero
    if (value->getType()->isFloatingPointTy()) {
        return builder_->CreateFCmpONE(
            value,
            llvm::ConstantFP::get(value->getType(), 0.0));
    }
    
    // If the value is a pointer, compare it with null
    if (value->getType()->isPointerTy()) {
        return builder_->CreateICmpNE(
            value,
            llvm::ConstantPointerNull::get(
                llvm::cast<llvm::PointerType>(value->getType())));
    }
    
    std::cerr << "Cannot convert value to boolean" << std::endl;
    return llvm::ConstantInt::getFalse(*context_);
}

/**
 * GetLValue - Get the address of an expression for assignment
 */
llvm::Value* CodeGenerator::GetLValue(Expr* expr) {
    if (auto var_expr = dynamic_cast<VarExpr*>(expr)) {
        const std::string& name = var_expr->GetName();
        
        // Look up the variable in the symbol table
        auto it = named_values_.find(name);
        if (it == named_values_.end()) {
            std::cerr << "Unknown variable name: " << name << std::endl;
            return nullptr;
        }
        
        return it->second;
    }
    else if (auto subscript_expr = dynamic_cast<SubscriptExpr*>(expr)) {
        // Visit the array/pointer
        subscript_expr->GetArray()->Accept(this);
        llvm::Value* array = value_stack_.top();
        value_stack_.pop();
        
        // Visit the index
        subscript_expr->GetIndex()->Accept(this);
        llvm::Value* index = value_stack_.top();
        value_stack_.pop();
        
        // Calculate the element address
        std::vector<llvm::Value*> indices;
        indices.push_back(index);
        
        return builder_->CreateGEP(
            ConvertType(subscript_expr->GetType()),
            array,
            indices,
            "elemptr");
    }
    else if (auto unary_expr = dynamic_cast<UnaryExpr*>(expr)) {
        if (unary_expr->GetOp() == UnaryExpr::Op::DEREF) {
            // Visit the operand
            unary_expr->GetOperand()->Accept(this);
            return value_stack_.top();
        }
    }
    
    std::cerr << "Expression is not an lvalue" << std::endl;
    return nullptr;
}

/**
 * EmitLogicalAnd - Emit code for short-circuit logical AND
 */
llvm::Value* CodeGenerator::EmitLogicalAnd(llvm::Value* lhs, llvm::Value* rhs) {
    // Convert LHS to boolean
    llvm::Value* lhs_bool = ConvertToBoolean(lhs);
    
    // Create the basic blocks
    llvm::Function* func = builder_->GetInsertBlock()->getParent();
    llvm::BasicBlock* rhs_block = llvm::BasicBlock::Create(*context_, "and_rhs", func);
    llvm::BasicBlock* end_block = llvm::BasicBlock::Create(*context_, "and_end", func);
    
    // Branch to the RHS block if LHS is true, otherwise short-circuit
    builder_->CreateCondBr(lhs_bool, rhs_block, end_block);
    
    // Emit the RHS block
    builder_->SetInsertPoint(rhs_block);
    llvm::Value* rhs_bool = ConvertToBoolean(rhs);
    builder_->CreateBr(end_block);
    
    // Emit the end block
    builder_->SetInsertPoint(end_block);
    
    // Create a PHI node to compute the result
    llvm::PHINode* result = builder_->CreatePHI(
        llvm::Type::getInt1Ty(*context_),
        2,
        "andtmp");
    
    result->addIncoming(
        llvm::ConstantInt::getFalse(*context_),
        builder_->GetInsertBlock()->getParent()->getEntryBlock().getFirstNonPHI()->getParent());
    
    result->addIncoming(rhs_bool, rhs_block);
    
    return result;
}

/**
 * EmitLogicalOr - Emit code for short-circuit logical OR
 */
llvm::Value* CodeGenerator::EmitLogicalOr(llvm::Value* lhs, llvm::Value* rhs) {
    // Convert LHS to boolean
    llvm::Value* lhs_bool = ConvertToBoolean(lhs);
    
    // Create the basic blocks
    llvm::Function* func = builder_->GetInsertBlock()->getParent();
    llvm::BasicBlock* rhs_block = llvm::BasicBlock::Create(*context_, "or_rhs", func);
    llvm::BasicBlock* end_block = llvm::BasicBlock::Create(*context_, "or_end", func);
    
    // Branch to the RHS block if LHS is false, otherwise short-circuit
    builder_->CreateCondBr(lhs_bool, end_block, rhs_block);
    
    // Emit the RHS block
    builder_->SetInsertPoint(rhs_block);
    llvm::Value* rhs_bool = ConvertToBoolean(rhs);
    builder_->CreateBr(end_block);
    
    // Emit the end block
    builder_->SetInsertPoint(end_block);
    
    // Create a PHI node to compute the result
    llvm::PHINode* result = builder_->CreatePHI(
        llvm::Type::getInt1Ty(*context_),
        2,
        "ortmp");
    
    result->addIncoming(
        llvm::ConstantInt::getTrue(*context_),
        builder_->GetInsertBlock()->getParent()->getEntryBlock().getFirstNonPHI()->getParent());
    
    result->addIncoming(rhs_bool, rhs_block);
    
    return result;
}

/**
 * EmitPreIncrement - Emit code for pre-increment (++x)
 */
llvm::Value* CodeGenerator::EmitPreIncrement(Expr* expr, llvm::Value* value) {
    // Get the lvalue
    llvm::Value* lvalue = GetLValue(expr);
    
    // Increment the value
    llvm::Value* one = llvm::ConstantInt::get(value->getType(), 1);
    llvm::Value* new_value = builder_->CreateAdd(value, one, "inc");
    
    // Store the new value
    builder_->CreateStore(new_value, lvalue);
    
    // Return the new value
    return new_value;
}

/**
 * EmitPreDecrement - Emit code for pre-decrement (--x)
 */
llvm::Value* CodeGenerator::EmitPreDecrement(Expr* expr, llvm::Value* value) {
    // Get the lvalue
    llvm::Value* lvalue = GetLValue(expr);
    
    // Decrement the value
    llvm::Value* one = llvm::ConstantInt::get(value->getType(), 1);
    llvm::Value* new_value = builder_->CreateSub(value, one, "dec");
    
    // Store the new value
    builder_->CreateStore(new_value, lvalue);
    
    // Return the new value
    return new_value;
}

/**
 * EmitPostIncrement - Emit code for post-increment (x++)
 */
llvm::Value* CodeGenerator::EmitPostIncrement(Expr* expr, llvm::Value* value) {
    // Get the lvalue
    llvm::Value* lvalue = GetLValue(expr);
    
    // Save the old value
    llvm::Value* old_value = value;
    
    // Increment the value
    llvm::Value* one = llvm::ConstantInt::get(value->getType(), 1);
    llvm::Value* new_value = builder_->CreateAdd(value, one, "inc");
    
    // Store the new value
    builder_->CreateStore(new_value, lvalue);
    
    // Return the old value
    return old_value;
}

/**
 * EmitPostDecrement - Emit code for post-decrement (x--)
 */
llvm::Value* CodeGenerator::EmitPostDecrement(Expr* expr, llvm::Value* value) {
    // Get the lvalue
    llvm::Value* lvalue = GetLValue(expr);
    
    // Save the old value
    llvm::Value* old_value = value;
    
    // Decrement the value
    llvm::Value* one = llvm::ConstantInt::get(value->getType(), 1);
    llvm::Value* new_value = builder_->CreateSub(value, one, "dec");
    
    // Store the new value
    builder_->CreateStore(new_value, lvalue);
    
    // Return the old value
    return old_value;
}

/**
 * IsFloatingPointType - Check if a type is a floating point type
 */
bool CodeGenerator::IsFloatingPointType(const std::shared_ptr<Type>& type) {
    return type->GetKind() == Type::Kind::FLOAT || 
           type->GetKind() == Type::Kind::DOUBLE;
}

/**
 * IsUnsignedType - Check if a type is an unsigned integer type
 */
bool CodeGenerator::IsUnsignedType(const std::shared_ptr<Type>& type) {
    // Check if this is a PrimitiveType with unsigned sign kind
    if (auto prim_type = std::dynamic_pointer_cast<PrimitiveType>(type)) {
        return prim_type->IsUnsigned();
    }
    return false;
}

/**
 * IsIntegerType - Check if a type is an integer type
 */
bool CodeGenerator::IsIntegerType(const std::shared_ptr<Type>& type) {
    return type->GetKind() == Type::Kind::BOOL || 
           type->GetKind() == Type::Kind::CHAR || 
           type->GetKind() == Type::Kind::SHORT || 
           type->GetKind() == Type::Kind::INT || 
           type->GetKind() == Type::Kind::LONG ||
           type->IsEnum(); // Enums are also integer types
}

/**
 * GetTypeSize - Get the size of a type in bits
 */
unsigned CodeGenerator::GetTypeSize(const std::shared_ptr<Type>& type) {
    switch (type->GetKind()) {
        case Type::Kind::BOOL:
            return 1;
            
        case Type::Kind::CHAR:
            return 8;
            
        case Type::Kind::SHORT:
            return 16;
            
        case Type::Kind::INT:
            return 32;
            
        case Type::Kind::LONG:
            return 64;
            
        case Type::Kind::FLOAT:
            return 32;
            
        case Type::Kind::DOUBLE:
            return 64;
            
        default:
            std::cerr << "Cannot get size of type: " << std::to_string(static_cast<int>(type->GetKind())) << std::endl;
            return 0;
    }
}

//===----------------------------------------------------------------------===//
// ASTVisitor implementation - Expressions
//===----------------------------------------------------------------------===//

/**
 * VisitBinaryExpr - Visit a binary expression node
 */
void CodeGenerator::VisitBinaryExpr(BinaryExpr* expr) {
    // Visit the operands
    expr->GetLeft()->Accept(this);
    auto L = value_stack_.top();
    value_stack_.pop();
    
    expr->GetRight()->Accept(this);
    auto R = value_stack_.top();
    value_stack_.pop();
    
    // Generate code based on the operator
    llvm::Value* result = nullptr;
    
    switch (expr->GetOp()) {
        case BinaryExpr::Op::ADD:
            if (IsFloatingPointType(expr->GetLeft()->GetType())) {
                result = builder_->CreateFAdd(L, R, "addtmp");
            } else {
                result = builder_->CreateAdd(L, R, "addtmp");
            }
            break;
            
        case BinaryExpr::Op::SUB:
            if (IsFloatingPointType(expr->GetLeft()->GetType())) {
                result = builder_->CreateFSub(L, R, "subtmp");
            } else {
                result = builder_->CreateSub(L, R, "subtmp");
            }
            break;
            
        case BinaryExpr::Op::MUL:
            if (IsFloatingPointType(expr->GetLeft()->GetType())) {
                result = builder_->CreateFMul(L, R, "multmp");
            } else {
                result = builder_->CreateMul(L, R, "multmp");
            }
            break;
            
        case BinaryExpr::Op::DIV:
            if (IsFloatingPointType(expr->GetLeft()->GetType())) {
                result = builder_->CreateFDiv(L, R, "divtmp");
            } else if (IsUnsignedType(expr->GetLeft()->GetType())) {
                result = builder_->CreateUDiv(L, R, "divtmp");
            } else {
                result = builder_->CreateSDiv(L, R, "divtmp");
            }
            break;
            
        case BinaryExpr::Op::MOD:
            if (IsUnsignedType(expr->GetLeft()->GetType())) {
                result = builder_->CreateURem(L, R, "modtmp");
            } else {
                result = builder_->CreateSRem(L, R, "modtmp");
            }
            break;
            
        case BinaryExpr::Op::BIT_AND:
            result = builder_->CreateAnd(L, R, "andtmp");
            break;
            
        case BinaryExpr::Op::BIT_OR:
            result = builder_->CreateOr(L, R, "ortmp");
            break;
            
        case BinaryExpr::Op::BIT_XOR:
            result = builder_->CreateXor(L, R, "xortmp");
            break;
            
        case BinaryExpr::Op::SHIFT_LEFT:
            result = builder_->CreateShl(L, R, "shltmp");
            break;
            
        case BinaryExpr::Op::SHIFT_RIGHT:
            if (IsUnsignedType(expr->GetLeft()->GetType())) {
                result = builder_->CreateLShr(L, R, "shrtmp");
            } else {
                result = builder_->CreateAShr(L, R, "shrtmp");
            }
            break;
            
        case BinaryExpr::Op::EQUAL:
            if (IsFloatingPointType(expr->GetLeft()->GetType())) {
                result = builder_->CreateFCmpOEQ(L, R, "eqtmp");
            } else {
                result = builder_->CreateICmpEQ(L, R, "eqtmp");
            }
            break;
            
        case BinaryExpr::Op::NOT_EQUAL:
            if (IsFloatingPointType(expr->GetLeft()->GetType())) {
                result = builder_->CreateFCmpONE(L, R, "netmp");
            } else {
                result = builder_->CreateICmpNE(L, R, "netmp");
            }
            break;
            
        case BinaryExpr::Op::LESS:
            if (IsFloatingPointType(expr->GetLeft()->GetType())) {
                result = builder_->CreateFCmpOLT(L, R, "lttmp");
            } else if (IsUnsignedType(expr->GetLeft()->GetType())) {
                result = builder_->CreateICmpULT(L, R, "lttmp");
            } else {
                result = builder_->CreateICmpSLT(L, R, "lttmp");
            }
            break;
            
        case BinaryExpr::Op::GREATER:
            if (IsFloatingPointType(expr->GetLeft()->GetType())) {
                result = builder_->CreateFCmpOGT(L, R, "gttmp");
            } else if (IsUnsignedType(expr->GetLeft()->GetType())) {
                result = builder_->CreateICmpUGT(L, R, "gttmp");
            } else {
                result = builder_->CreateICmpSGT(L, R, "gttmp");
            }
            break;
            
        case BinaryExpr::Op::LESS_EQUAL:
            if (IsFloatingPointType(expr->GetLeft()->GetType())) {
                result = builder_->CreateFCmpOLE(L, R, "letmp");
            } else if (IsUnsignedType(expr->GetLeft()->GetType())) {
                result = builder_->CreateICmpULE(L, R, "letmp");
            } else {
                result = builder_->CreateICmpSLE(L, R, "letmp");
            }
            break;
            
        case BinaryExpr::Op::GREATER_EQUAL:
            if (IsFloatingPointType(expr->GetLeft()->GetType())) {
                result = builder_->CreateFCmpOGE(L, R, "getmp");
            } else if (IsUnsignedType(expr->GetLeft()->GetType())) {
                result = builder_->CreateICmpUGE(L, R, "getmp");
            } else {
                result = builder_->CreateICmpSGE(L, R, "getmp");
            }
            break;
            
        case BinaryExpr::Op::LOGICAL_AND:
            // Short-circuit evaluation for logical AND
            result = EmitLogicalAnd(L, R);
            break;
            
        case BinaryExpr::Op::LOGICAL_OR:
            // Short-circuit evaluation for logical OR
            result = EmitLogicalOr(L, R);
            break;
    }
    
    value_stack_.push(result);
}

/**
 * VisitUnaryExpr - Visit a unary expression node
 */
void CodeGenerator::VisitUnaryExpr(UnaryExpr* expr) {
    // Visit the operand
    expr->GetOperand()->Accept(this);
    llvm::Value* operand = value_stack_.top();
    value_stack_.pop();
    
    llvm::Value* result = nullptr;
    
    switch (expr->GetOp()) {
        case UnaryExpr::Op::NEGATE:
            if (IsFloatingPointType(expr->GetOperand()->GetType())) {
                result = builder_->CreateFNeg(operand, "negtmp");
            } else {
                result = builder_->CreateNeg(operand, "negtmp");
            }
            break;
            
        case UnaryExpr::Op::NOT:
            result = builder_->CreateNot(operand, "nottmp");
            break;
            
        case UnaryExpr::Op::LOGICAL_NOT:
            result = builder_->CreateNot(ConvertToBoolean(operand), "lnottmp");
            break;
            
        case UnaryExpr::Op::PRE_INC: {
            // Get the raw pointer from the shared_ptr
            Expr* operandExpr = expr->GetOperand().get();
            result = EmitPreIncrement(operandExpr, operand);
            break;
        }
            
        case UnaryExpr::Op::PRE_DEC: {
            // Get the raw pointer from the shared_ptr
            Expr* operandExpr = expr->GetOperand().get();
            result = EmitPreDecrement(operandExpr, operand);
            break;
        }
            
        case UnaryExpr::Op::POST_INC: {
            // Get the raw pointer from the shared_ptr
            Expr* operandExpr = expr->GetOperand().get();
            result = EmitPostIncrement(operandExpr, operand);
            break;
        }
            
        case UnaryExpr::Op::POST_DEC: {
            // Get the raw pointer from the shared_ptr
            Expr* operandExpr = expr->GetOperand().get();
            result = EmitPostDecrement(operandExpr, operand);
            break;
        }
            
        case UnaryExpr::Op::ADDR:
            // Get the address of the operand
            result = GetLValue(expr->GetOperand().get());
            break;
            
        case UnaryExpr::Op::DEREF:
            // Operand is a pointer, load its value
            result = builder_->CreateLoad(
                ConvertType(expr->GetType()),
                operand,
                "deref");
            break;
    }
    
    value_stack_.push(result);
}

/**
 * VisitLiteralExpr - Visit a literal expression node
 */
void CodeGenerator::VisitLiteralExpr(LiteralExpr* expr) {
    llvm::Value* result = nullptr;
    
    switch (expr->GetLiteralKind()) {
        case LiteralExpr::Kind::BOOL:
            result = expr->GetBoolValue() ? 
                llvm::ConstantInt::getTrue(*context_) : 
                llvm::ConstantInt::getFalse(*context_);
            break;
            
        case LiteralExpr::Kind::INT:
            result = llvm::ConstantInt::get(
                ConvertType(expr->GetType()),
                expr->GetIntValue());
            break;
            
        case LiteralExpr::Kind::FLOAT:
            result = llvm::ConstantFP::get(
                ConvertType(expr->GetType()),
                expr->GetFloatValue());
            break;
            
        case LiteralExpr::Kind::CHAR:
            result = llvm::ConstantInt::get(
                llvm::Type::getInt8Ty(*context_),
                expr->GetCharValue());
            break;
            
        case LiteralExpr::Kind::STRING: {
            // Get the string value
            const std::string& str = expr->GetStringValue();
            
            // Create a global constant for the string
            llvm::Constant* strConstant = llvm::ConstantDataArray::getString(
                *context_,
                str,
                true);
            
            // Create a global variable to hold the string
            llvm::GlobalVariable* globalStr = new llvm::GlobalVariable(
                *module_,
                strConstant->getType(),
                true,
                llvm::GlobalValue::PrivateLinkage,
                strConstant,
                ".str");
            
            // Get a pointer to the first element of the string
            llvm::Value* zero = llvm::ConstantInt::get(
                llvm::Type::getInt32Ty(*context_),
                0);
            
            std::vector<llvm::Value*> indices;
            indices.push_back(zero);
            indices.push_back(zero);
            
            result = builder_->CreateGEP(
                globalStr->getValueType(),
                globalStr,
                indices,
                "strptr");
            break;
        }
            
        case LiteralExpr::Kind::NULL_PTR:
            result = llvm::ConstantPointerNull::get(
                llvm::cast<llvm::PointerType>(ConvertType(expr->GetType())));
            break;
    }
    
    value_stack_.push(result);
}

/**
 * VisitVarExpr - Visit a variable expression node
 */
void CodeGenerator::VisitVarExpr(VarExpr* expr) {
    // Look up the variable in the symbol table
    const std::string& name = expr->GetName();
    llvm::Value* lvalue = nullptr;
    
    auto it = named_values_.find(name);
    if (it == named_values_.end()) {
        std::cerr << "Unknown variable name: " << name << std::endl;
        return;
    }
    
    lvalue = it->second;
    
    // Load the variable's value
    llvm::Value* value = builder_->CreateLoad(
        ConvertType(expr->GetType()),
        lvalue,
        name.c_str());
    
    value_stack_.push(value);
}

/**
 * VisitAssignExpr - Visit an assignment expression node
 */
void CodeGenerator::VisitAssignExpr(AssignExpr* expr) {
    // Get the address of the target
    llvm::Value* lvalue = GetLValue(expr->GetTarget().get());
    
    // Evaluate the value to be assigned
    expr->GetValue()->Accept(this);
    llvm::Value* rvalue = value_stack_.top();
    value_stack_.pop();
    
    // Store the value
    builder_->CreateStore(rvalue, lvalue);
    
    // The result of an assignment is the assigned value
    value_stack_.push(rvalue);
}

/**
 * VisitCallExpr - Visit a function call expression node
 */
void CodeGenerator::VisitCallExpr(CallExpr* expr) {
    // Get the function
    llvm::Function* callee = module_->getFunction(expr->GetCallee());
    
    if (!callee) {
        std::cerr << "Unknown function: " << expr->GetCallee() << std::endl;
        return;
    }
    
    // Check number of arguments
    if (callee->arg_size() != expr->GetArgs().size() && !callee->isVarArg()) {
        std::cerr << "Incorrect number of arguments to function: " << expr->GetCallee() << std::endl;
        return;
    }
    
    // Evaluate the arguments
    std::vector<llvm::Value*> args;
    for (const auto& arg : expr->GetArgs()) {
        arg->Accept(this);
        args.push_back(value_stack_.top());
        value_stack_.pop();
    }
    
    // Call the function
    llvm::Value* result = builder_->CreateCall(callee, args, "calltmp");
    
    value_stack_.push(result);
}

/**
 * VisitMessageExpr - Visit an Objective-C style message expression node
 */
void CodeGenerator::VisitMessageExpr(MessageExpr* expr) {
    // Get the receiver object
    expr->GetReceiver()->Accept(this);
    llvm::Value* receiver = value_stack_.top();
    value_stack_.pop();
    
    // Get the method name (selector)
    std::string selector = expr->GetSelector();
    
    // Transform the message syntax into a function call
    // For example, [obj foo:x bar:y] -> foo_bar(obj, x, y)
    std::string funcName = selector;
    std::replace(funcName.begin(), funcName.end(), ':', '_');
    
    // Get the function
    llvm::Function* callee = module_->getFunction(funcName);
    
    if (!callee) {
        std::cerr << "Unknown method: " << selector << std::endl;
        return;
    }
    
    // Build the argument list, starting with the receiver
    std::vector<llvm::Value*> args;
    args.push_back(receiver);
    
    // Add the remaining arguments
    for (const auto& arg : expr->GetArgs()) {
        arg->Accept(this);
        args.push_back(value_stack_.top());
        value_stack_.pop();
    }
    
    // Call the function
    llvm::Value* result = builder_->CreateCall(callee, args, "msgtmp");
    
    value_stack_.push(result);
}

/**
 * VisitSubscriptExpr - Visit a subscript expression node
 */
void CodeGenerator::VisitSubscriptExpr(SubscriptExpr* expr) {
    // Visit the array/pointer
    expr->GetArray()->Accept(this);
    llvm::Value* array = value_stack_.top();
    value_stack_.pop();
    
    // Visit the index
    expr->GetIndex()->Accept(this);
    llvm::Value* index = value_stack_.top();
    value_stack_.pop();
    
    // Calculate the element address
    std::vector<llvm::Value*> indices;
    indices.push_back(index);
    
    llvm::Value* elemPtr = builder_->CreateGEP(
        ConvertType(expr->GetType()),
        array,
        indices,
        "elemptr");
    
    // Load the element
    llvm::Value* elem = builder_->CreateLoad(
        ConvertType(expr->GetType()),
        elemPtr,
        "elem");
    
    value_stack_.push(elem);
}

/**
 * VisitCastExpr - Visit a cast expression node
 */
void CodeGenerator::VisitCastExpr(CastExpr* expr) {
    // Visit the operand
    expr->GetExpr()->Accept(this);
    llvm::Value* operand = value_stack_.top();
    value_stack_.pop();
    
    // Get the source and target types
    std::shared_ptr<Type> src_type = expr->GetExpr()->GetType();
    std::shared_ptr<Type> dst_type = expr->GetType();
    
    llvm::Value* result = nullptr;
    
    // Handle the cast based on the source and target types
    if (IsIntegerType(src_type) && IsIntegerType(dst_type)) {
        // Integer to integer cast
        unsigned src_size = GetTypeSize(src_type);
        unsigned dst_size = GetTypeSize(dst_type);
        
        if (src_size == dst_size) {
            // No need to cast, just use the operand
            result = operand;
        } else if (src_size < dst_size) {
            // Extension
            if (IsUnsignedType(src_type)) {
                result = builder_->CreateZExt(operand, ConvertType(dst_type), "zexttmp");
            } else {
                result = builder_->CreateSExt(operand, ConvertType(dst_type), "sexttmp");
            }
        } else {
            // Truncation
            result = builder_->CreateTrunc(operand, ConvertType(dst_type), "trunctmp");
        }
    } else if (IsIntegerType(src_type) && IsFloatingPointType(dst_type)) {
        // Integer to floating point cast
        if (IsUnsignedType(src_type)) {
            result = builder_->CreateUIToFP(operand, ConvertType(dst_type), "uitofptmp");
        } else {
            result = builder_->CreateSIToFP(operand, ConvertType(dst_type), "sitofptmp");
        }
    } else if (IsFloatingPointType(src_type) && IsIntegerType(dst_type)) {
        // Floating point to integer cast
        if (IsUnsignedType(dst_type)) {
            result = builder_->CreateFPToUI(operand, ConvertType(dst_type), "fptouitmp");
        } else {
            result = builder_->CreateFPToSI(operand, ConvertType(dst_type), "fptositmp");
        }
    } else if (IsFloatingPointType(src_type) && IsFloatingPointType(dst_type)) {
        // Floating point to floating point cast
        unsigned src_size = GetTypeSize(src_type);
        unsigned dst_size = GetTypeSize(dst_type);
        
        if (src_size == dst_size) {
            // No need to cast, just use the operand
            result = operand;
        } else if (src_size < dst_size) {
            // Extension
            result = builder_->CreateFPExt(operand, ConvertType(dst_type), "fpexttmp");
        } else {
            // Truncation
            result = builder_->CreateFPTrunc(operand, ConvertType(dst_type), "fptrunctmp");
        }
    } else if (src_type->GetKind() == Type::Kind::POINTER && dst_type->GetKind() == Type::Kind::POINTER) {
        // Pointer to pointer cast
        result = builder_->CreateBitCast(operand, ConvertType(dst_type), "ptrcatttmp");
    } else if (IsIntegerType(src_type) && dst_type->GetKind() == Type::Kind::POINTER) {
        // Integer to pointer cast
        result = builder_->CreateIntToPtr(operand, ConvertType(dst_type), "inttoptr");
    } else if (src_type->GetKind() == Type::Kind::POINTER && IsIntegerType(dst_type)) {
        // Pointer to integer cast
        result = builder_->CreatePtrToInt(operand, ConvertType(dst_type), "ptrtoint");
    } else {
        std::cerr << "Unsupported cast from " << std::to_string(static_cast<int>(src_type->GetKind())) 
                  << " to " << std::to_string(static_cast<int>(dst_type->GetKind())) << std::endl;
        return;
    }
    
    value_stack_.push(result);
}

//===----------------------------------------------------------------------===//
// ASTVisitor implementation - Statements
//===----------------------------------------------------------------------===//

/**
 * VisitExprStmt - Visit an expression statement node
 */
void CodeGenerator::VisitExprStmt(ExprStmt* stmt) {
    // Visit the expression
    stmt->GetExpr()->Accept(this);
    
    // Pop the result, we don't need it
    value_stack_.pop();
}

/**
 * VisitBlockStmt - Visit a block statement node
 */
void CodeGenerator::VisitBlockStmt(BlockStmt* stmt) {
    // Create a new scope for the block
    BeginScope();
    
    // Generate code for each statement in the block
    for (const auto& s : stmt->GetStmts()) {
        s->Accept(this);
    }
    
    // End the scope for the block
    EndScope();
}

/**
 * VisitIfStmt - Visit an if statement node
 */
void CodeGenerator::VisitIfStmt(IfStmt* stmt) {
    // Visit the condition
    stmt->GetCond()->Accept(this);
    llvm::Value* cond_val = value_stack_.top();
    value_stack_.pop();
    
    // Convert condition to a boolean value
    llvm::Value* cond_bool = ConvertToBoolean(cond_val);
    
    // Create basic blocks for then, else, and merge
    llvm::Function* func = builder_->GetInsertBlock()->getParent();
    llvm::BasicBlock* then_bb = llvm::BasicBlock::Create(*context_, "then", func);
    llvm::BasicBlock* else_bb = llvm::BasicBlock::Create(*context_, "else");
    llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create(*context_, "ifcont");
    
    if (stmt->GetElse()) {
        // If there is an else clause, branch to either then_bb or else_bb
        builder_->CreateCondBr(cond_bool, then_bb, else_bb);
    } else {
        // Otherwise, branch to either then_bb or merge_bb
        builder_->CreateCondBr(cond_bool, then_bb, merge_bb);
    }
    
    // Emit then block
    builder_->SetInsertPoint(then_bb);
    stmt->GetThen()->Accept(this);
    
    // Branch to merge block (if not already terminated)
    if (!builder_->GetInsertBlock()->getTerminator()) {
        builder_->CreateBr(merge_bb);
    }
    
    // Emit else block if it exists
    if (stmt->GetElse()) {
        else_bb->insertInto(func);
        builder_->SetInsertPoint(else_bb);
        stmt->GetElse()->Accept(this);
        
        // Branch to merge block (if not already terminated)
        if (!builder_->GetInsertBlock()->getTerminator()) {
            builder_->CreateBr(merge_bb);
        }
    }
    
    // Emit merge block
    merge_bb->insertInto(func);
    builder_->SetInsertPoint(merge_bb);
}

/**
 * VisitWhileStmt - Visit a while statement node
 */
void CodeGenerator::VisitWhileStmt(WhileStmt* stmt) {
    // Create basic blocks for the loop condition, body, and end
    llvm::Function* func = builder_->GetInsertBlock()->getParent();
    llvm::BasicBlock* cond_bb = llvm::BasicBlock::Create(*context_, "while.cond", func);
    llvm::BasicBlock* body_bb = llvm::BasicBlock::Create(*context_, "while.body");
    llvm::BasicBlock* end_bb = llvm::BasicBlock::Create(*context_, "while.end");
    
    // Branch to the condition block
    builder_->CreateBr(cond_bb);
    
    // Save the old break and continue targets
    llvm::BasicBlock* old_break = break_target_;
    llvm::BasicBlock* old_continue = continue_target_;
    
    // Set the new break and continue targets
    break_target_ = end_bb;
    continue_target_ = cond_bb;
    
    // Emit the condition block
    builder_->SetInsertPoint(cond_bb);
    stmt->GetCond()->Accept(this);
    llvm::Value* cond_val = value_stack_.top();
    value_stack_.pop();
    
    // Convert condition to a boolean value
    llvm::Value* cond_bool = ConvertToBoolean(cond_val);
    
    // Branch to the body if condition is true, otherwise to the end
    builder_->CreateCondBr(cond_bool, body_bb, end_bb);
    
    // Emit the body block
    body_bb->insertInto(func);
    builder_->SetInsertPoint(body_bb);
    stmt->GetBody()->Accept(this);
    
    // Branch back to the condition block (if not already terminated)
    if (!builder_->GetInsertBlock()->getTerminator()) {
        builder_->CreateBr(cond_bb);
    }
    
    // Emit the end block
    end_bb->insertInto(func);
    builder_->SetInsertPoint(end_bb);
    
    // Restore the old break and continue targets
    break_target_ = old_break;
    continue_target_ = old_continue;
}

/**
 * VisitForStmt - Visit a for statement node
 */
void CodeGenerator::VisitForStmt(ForStmt* stmt) {
    // Create a new scope for the for statement
    BeginScope();
    
    // Create basic blocks for the loop initialization, condition, increment, body, and end
    llvm::Function* func = builder_->GetInsertBlock()->getParent();
    llvm::BasicBlock* init_bb = builder_->GetInsertBlock();
    llvm::BasicBlock* cond_bb = llvm::BasicBlock::Create(*context_, "for.cond", func);
    llvm::BasicBlock* body_bb = llvm::BasicBlock::Create(*context_, "for.body");
    llvm::BasicBlock* inc_bb = llvm::BasicBlock::Create(*context_, "for.inc");
    llvm::BasicBlock* end_bb = llvm::BasicBlock::Create(*context_, "for.end");
    
    // Emit the initialization
    if (stmt->GetInit()) {
        stmt->GetInit()->Accept(this);
    }
    
    // Branch to the condition block
    builder_->CreateBr(cond_bb);
    
    // Save the old break and continue targets
    llvm::BasicBlock* old_break = break_target_;
    llvm::BasicBlock* old_continue = continue_target_;
    
    // Set the new break and continue targets
    break_target_ = end_bb;
    continue_target_ = inc_bb;
    
    // Emit the condition block
    builder_->SetInsertPoint(cond_bb);
    if (stmt->GetCond()) {
        stmt->GetCond()->Accept(this);
        llvm::Value* cond_val = value_stack_.top();
        value_stack_.pop();
        
        // Convert condition to a boolean value
        llvm::Value* cond_bool = ConvertToBoolean(cond_val);
        
        // Branch to the body if condition is true, otherwise to the end
        builder_->CreateCondBr(cond_bool, body_bb, end_bb);
    } else {
        // No condition, always branch to the body
        builder_->CreateBr(body_bb);
    }
    
    // Emit the body block
    body_bb->insertInto(func);
    builder_->SetInsertPoint(body_bb);
    stmt->GetBody()->Accept(this);
    
    // Branch to the increment block (if not already terminated)
    if (!builder_->GetInsertBlock()->getTerminator()) {
        builder_->CreateBr(inc_bb);
    }
    
    // Emit the increment block
    inc_bb->insertInto(func);
    builder_->SetInsertPoint(inc_bb);
    if (stmt->GetInc()) {
        stmt->GetInc()->Accept(this);
        value_stack_.pop(); // Pop the result, we don't need it
    }
    
    // Branch back to the condition block
    builder_->CreateBr(cond_bb);
    
    // Emit the end block
    end_bb->insertInto(func);
    builder_->SetInsertPoint(end_bb);
    
    // Restore the old break and continue targets
    break_target_ = old_break;
    continue_target_ = old_continue;
    
    // End the scope for the for statement
    EndScope();
}

/**
 * VisitBreakStmt - Visit a break statement node
 */
void CodeGenerator::VisitBreakStmt(BreakStmt* stmt) {
    if (!break_target_) {
        std::cerr << "Break statement outside of loop" << std::endl;
        return;
    }
    
    builder_->CreateBr(break_target_);
}

/**
 * VisitContinueStmt - Visit a continue statement node
 */
void CodeGenerator::VisitContinueStmt(ContinueStmt* stmt) {
    if (!continue_target_) {
        std::cerr << "Continue statement outside of loop" << std::endl;
        return;
    }
    
    builder_->CreateBr(continue_target_);
}

/**
 * VisitReturnStmt - Visit a return statement node
 */
void CodeGenerator::VisitReturnStmt(ReturnStmt* stmt) {
    if (!current_function_) {
        std::cerr << "Return statement outside of function" << std::endl;
        return;
    }
    
    if (stmt->GetExpr()) {
        // Return value
        stmt->GetExpr()->Accept(this);
        llvm::Value* ret_val = value_stack_.top();
        value_stack_.pop();
        
        builder_->CreateRet(ret_val);
    } else {
        // Void return
        builder_->CreateRetVoid();
    }
}

/**
 * VisitDeclStmt - Visit a declaration statement node
 */
void CodeGenerator::VisitDeclStmt(DeclStmt* stmt) {
    // Visit the declaration
    stmt->GetDecl()->Accept(this);
}

//===----------------------------------------------------------------------===//
// ASTVisitor implementation - Declarations
//===----------------------------------------------------------------------===//

/**
 * VisitVarDecl - Visit a variable declaration node
 */
void CodeGenerator::VisitVarDecl(VarDecl* decl) {
    const std::string& name = decl->GetName();
    std::shared_ptr<Type> type = decl->GetType();
    
    // Create an alloca for the variable
    llvm::Function* func = builder_->GetInsertBlock()->getParent();
    llvm::IRBuilder<> entry_builder(&func->getEntryBlock(), func->getEntryBlock().begin());
    
    llvm::AllocaInst* alloca = entry_builder.CreateAlloca(
        ConvertType(type),
        nullptr,
        name.c_str());
    
    // Add the variable to the symbol table
    named_values_[name] = alloca;
    scopes_.back().values[name] = alloca;
    
    // Initialize the variable if an initializer is provided
    if (decl->GetInit()) {
        decl->GetInit()->Accept(this);
        llvm::Value* init_val = value_stack_.top();
        value_stack_.pop();
        
        builder_->CreateStore(init_val, alloca);
    }
}

/**
 * VisitParamDecl - Visit a parameter declaration node
 */
void CodeGenerator::VisitParamDecl(ParamDecl* decl) {
    // Parameters should already be handled by VisitFuncDecl
}

/**
 * VisitFuncDecl - Visit a function declaration node
 */
void CodeGenerator::VisitFuncDecl(FuncDecl* decl) {
    const std::string& name = decl->GetName();
    std::shared_ptr<FunctionType> func_type = std::static_pointer_cast<FunctionType>(decl->GetType());
    
    // Get the return type and parameter types
    llvm::Type* return_type = ConvertType(func_type->GetReturnType());
    
    std::vector<llvm::Type*> param_types;
    for (const auto& param_type : func_type->GetParamTypes()) {
        param_types.push_back(ConvertType(param_type));
    }
    
    // Create the function type
    llvm::FunctionType* llvm_func_type = llvm::FunctionType::get(
        return_type,
        param_types,
                    func_type->IsVariadic());
    
    // Create the function
    llvm::Function* func = llvm::Function::Create(
        llvm_func_type,
        llvm::Function::ExternalLinkage,
        name,
        module_.get());
    
    // Add the function to the function table
    function_table_[name] = func;
    
    // Set parameter names
    unsigned idx = 0;
    for (auto& param : func->args()) {
        param.setName(decl->GetParams()[idx++]->GetName());
    }
    
    // If this is a declaration without a body, we're done
    if (!decl->GetBody()) {
        return;
    }
    
    // Create a new basic block to start insertion into
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(*context_, "entry", func);
    builder_->SetInsertPoint(bb);
    
    // Save the current function
    llvm::Function* prev_func = current_function_;
    current_function_ = func;
    
    // Create a new scope for the function body
    BeginScope();
    
    // Create allocas for the parameters
    for (auto& param : func->args()) {
        // Create an alloca for this variable
        llvm::AllocaInst* alloca = builder_->CreateAlloca(
            param.getType(),
            nullptr,
            param.getName());
        
        // Store the parameter value
        builder_->CreateStore(&param, alloca);
        
        // Add the variable to the symbol table
        named_values_[param.getName().str()] = alloca;
        scopes_.back().values[param.getName().str()] = alloca;
    }
    
    // Generate code for the function body
    decl->GetBody()->Accept(this);
    
    // If the block doesn't have a terminator (e.g., return), add one
    if (!builder_->GetInsertBlock()->getTerminator()) {
        if (return_type->isVoidTy()) {
            builder_->CreateRetVoid();
        } else {
            // Return a default value for non-void functions
            llvm::Value* default_val = nullptr;
            
            if (return_type->isIntegerTy()) {
                default_val = llvm::ConstantInt::get(return_type, 0);
            } else if (return_type->isFloatingPointTy()) {
                default_val = llvm::ConstantFP::get(return_type, 0.0);
            } else if (return_type->isPointerTy()) {
                default_val = llvm::ConstantPointerNull::get(
                    llvm::cast<llvm::PointerType>(return_type));
            } else {
                std::cerr << "Unsupported return type" << std::endl;
                return;
            }
            
            builder_->CreateRet(default_val);
        }
    }
    
    // End the scope for the function body
    EndScope();
    
    // Restore the previous function
    current_function_ = prev_func;
    
    // Verify the function
    if (llvm::verifyFunction(*func, &llvm::errs())) {
        func->eraseFromParent();
        std::cerr << "Function verification failed" << std::endl;
        return;
    }
}

/**
 * VisitMethodDecl - Visit a method declaration node
 */
void CodeGenerator::VisitMethodDecl(MethodDecl* decl) {
    const std::string& name = decl->GetName();
    std::shared_ptr<FunctionType> func_type = std::static_pointer_cast<FunctionType>(decl->GetType());
    
    // Transform the method name based on the Objective-C style syntax
    // For example, foo:bar: -> foo_bar
    std::string transformed_name = name;
    std::replace(transformed_name.begin(), transformed_name.end(), ':', '_');
    
    // Get the return type and parameter types
    llvm::Type* return_type = ConvertType(func_type->GetReturnType());
    
    std::vector<llvm::Type*> param_types;
    
    // Add the receiver type as the first parameter
    param_types.push_back(ConvertType(decl->GetReceiverType()));
    
    // Add the rest of the parameter types
    for (const auto& param_type : func_type->GetParamTypes()) {
        param_types.push_back(ConvertType(param_type));
    }
    
    // Create the function type
    llvm::FunctionType* llvm_func_type = llvm::FunctionType::get(
        return_type,
        param_types,
        func_type->IsVariadic());
    
    // Create the function
    llvm::Function* func = llvm::Function::Create(
        llvm_func_type,
        llvm::Function::ExternalLinkage,
        transformed_name,
        module_.get());
    
    // Add the function to the function table
    function_table_[transformed_name] = func;
    
    // Set parameter names
    auto arg_it = func->arg_begin();
    
    // The first parameter is the receiver
    arg_it->setName("self");
    ++arg_it;
    
    // The rest of the parameters match the method's parameters
    unsigned idx = 0;
    for (; arg_it != func->arg_end(); ++arg_it, ++idx) {
        arg_it->setName(decl->GetParams()[idx]->GetName());
    }
    
    // If this is a declaration without a body, we're done
    if (!decl->GetBody()) {
        return;
    }
    
    // Create a new basic block to start insertion into
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(*context_, "entry", func);
    builder_->SetInsertPoint(bb);
    
    // Save the current function
    llvm::Function* prev_func = current_function_;
    current_function_ = func;
    
    // Create a new scope for the function body
    BeginScope();
    
    // Create allocas for the parameters
    for (auto& param : func->args()) {
        // Create an alloca for this variable
        llvm::AllocaInst* alloca = builder_->CreateAlloca(
            param.getType(),
            nullptr,
            param.getName());
        
        // Store the parameter value
        builder_->CreateStore(&param, alloca);
        
        // Add the variable to the symbol table
        named_values_[param.getName().str()] = alloca;
        scopes_.back().values[param.getName().str()] = alloca;
    }
    
    // Generate code for the function body
    decl->GetBody()->Accept(this);
    
    // If the block doesn't have a terminator (e.g., return), add one
    if (!builder_->GetInsertBlock()->getTerminator()) {
        if (return_type->isVoidTy()) {
            builder_->CreateRetVoid();
        } else {
            // Return a default value for non-void functions
            llvm::Value* default_val = nullptr;
            
            if (return_type->isIntegerTy()) {
                default_val = llvm::ConstantInt::get(return_type, 0);
            } else if (return_type->isFloatingPointTy()) {
                default_val = llvm::ConstantFP::get(return_type, 0.0);
            } else if (return_type->isPointerTy()) {
                default_val = llvm::ConstantPointerNull::get(
                    llvm::cast<llvm::PointerType>(return_type));
            } else {
                std::cerr << "Unsupported return type" << std::endl;
                return;
            }
            
            builder_->CreateRet(default_val);
        }
    }
    
    // End the scope for the function body
    EndScope();
    
    // Restore the previous function
    current_function_ = prev_func;
    
    // Verify the function
    if (llvm::verifyFunction(*func, &llvm::errs())) {
        func->eraseFromParent();
        std::cerr << "Function verification failed" << std::endl;
        return;
    }
}

/**
 * VisitStructDecl - Visit a struct declaration node
 */
void CodeGenerator::VisitStructDecl(StructDecl* decl) {
    const std::string& name = decl->GetName();
    std::vector<std::shared_ptr<Type>> field_types;
    
    // Get the field types
    for (const auto& field : decl->GetFields()) {
        field_types.push_back(field->GetType());
    }
    
    // Create a struct type in LLVM IR
    std::vector<llvm::Type*> llvm_field_types;
    for (const auto& field_type : field_types) {
        llvm_field_types.push_back(ConvertType(field_type));
    }
    
    llvm::StructType* struct_type = llvm::StructType::create(
        *context_,
        llvm_field_types,
        name);
    
    // Add the struct type to the struct types table
    struct_types_[name] = struct_type;
}

/**
 * VisitEnumDecl - Visit an enum declaration node
 */
void CodeGenerator::VisitEnumDecl(EnumDecl* decl) {
    const std::string& name = decl->GetName();
    
    // Get the enum's base type (e.g., int)
    llvm::Type* base_type = ConvertType(decl->GetBaseType());
    
    // Define each enum value as a constant
    for (const auto& enum_val : decl->GetValues()) {
        llvm::Constant* value = llvm::ConstantInt::get(
            base_type,
            enum_val.second);
        
        // Create a global constant for the enum value
        new llvm::GlobalVariable(
            *module_,
            base_type,
            true, // is constant
            llvm::GlobalValue::InternalLinkage,
            value,
            name + "::" + enum_val.first);
    }
}

} // namespace dsLang
