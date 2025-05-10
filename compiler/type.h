/**
 * type.h - Type System for dsLang
 * 
 * This file defines the Type class hierarchy used to represent the types
 * of expressions and declarations in dsLang.
 */

#ifndef DSLANG_TYPE_H
#define DSLANG_TYPE_H

#include <memory>
#include <string>
#include <vector>

namespace dsLang {

/**
 * Type - Base class for all types
 */
class Type {
public:
    enum class Kind {
        VOID,
        BOOL,
        CHAR,
        SHORT,
        INT,
        LONG,
        FLOAT,
        DOUBLE,
        POINTER,
        ARRAY,
        STRUCT,
        ENUM,
        FUNCTION
    };
    
    /**
     * Constructor
     */
    Type(Kind kind) : kind_(kind) {}
    
    /**
     * Destructor
     */
    virtual ~Type() = default;
    
    /**
     * GetKind - Get the kind of type
     */
    Kind GetKind() const { return kind_; }
    
    /**
     * GetSize - Get the size of the type in bytes
     */
    virtual size_t GetSize() const = 0;
    
    /**
     * GetAlignment - Get the alignment of the type in bytes
     */
    virtual size_t GetAlignment() const = 0;
    
    /**
     * ToString - Convert the type to a string representation
     */
    virtual std::string ToString() const = 0;
    
    /**
     * IsEqual - Check if this type is equal to another type
     */
    virtual bool IsEqual(const Type* other) const;
    
    /**
     * IsVoid - Check if this is a void type
     */
    bool IsVoid() const { return kind_ == Kind::VOID; }
    
    /**
     * IsBool - Check if this is a bool type
     */
    bool IsBool() const { return kind_ == Kind::BOOL; }
    
    /**
     * IsChar - Check if this is a char type
     */
    bool IsChar() const { return kind_ == Kind::CHAR; }
    
    /**
     * IsShort - Check if this is a short type
     */
    bool IsShort() const { return kind_ == Kind::SHORT; }
    
    /**
     * IsInt - Check if this is an int type
     */
    bool IsInt() const { return kind_ == Kind::INT; }
    
    /**
     * IsLong - Check if this is a long type
     */
    bool IsLong() const { return kind_ == Kind::LONG; }
    
    /**
     * IsFloat - Check if this is a float type
     */
    bool IsFloat() const { return kind_ == Kind::FLOAT; }
    
    /**
     * IsDouble - Check if this is a double type
     */
    bool IsDouble() const { return kind_ == Kind::DOUBLE; }
    
    /**
     * IsPointer - Check if this is a pointer type
     */
    bool IsPointer() const { return kind_ == Kind::POINTER; }
    
    /**
     * IsArray - Check if this is an array type
     */
    bool IsArray() const { return kind_ == Kind::ARRAY; }
    
    /**
     * IsStruct - Check if this is a struct type
     */
    bool IsStruct() const { return kind_ == Kind::STRUCT; }
    
    /**
     * IsEnum - Check if this is an enum type
     */
    bool IsEnum() const { return kind_ == Kind::ENUM; }
    
    /**
     * IsFunction - Check if this is a function type
     */
    bool IsFunction() const { return kind_ == Kind::FUNCTION; }
    
    /**
     * IsIntegral - Check if this is an integral type (bool, char, short, int, long, enum)
     */
    bool IsIntegral() const;
    
    /**
     * IsFloatingPoint - Check if this is a floating-point type (float, double)
     */
    bool IsFloatingPoint() const;
    
    /**
     * IsArithmetic - Check if this is an arithmetic type (integral or floating-point)
     */
    bool IsArithmetic() const;
    
    /**
     * IsScalar - Check if this is a scalar type (arithmetic, pointer, enum)
     */
    bool IsScalar() const;
    
private:
    Kind kind_;
};

/**
 * VoidType - Represents the void type
 */
class VoidType : public Type {
public:
    /**
     * Constructor
     */
    VoidType() : Type(Kind::VOID) {}
    
    /**
     * GetSize - Get the size of the type in bytes (0 for void)
     */
    size_t GetSize() const override { return 0; }
    
    /**
     * GetAlignment - Get the alignment of the type in bytes
     */
    size_t GetAlignment() const override { return 0; }
    
    /**
     * ToString - Convert the type to a string representation
     */
    std::string ToString() const override { return "void"; }
};

/**
 * PrimitiveType - Base class for primitive types
 */
class PrimitiveType : public Type {
public:
    enum class SignKind {
        SIGNED,
        UNSIGNED
    };
    
    /**
     * Constructor
     */
    PrimitiveType(Kind kind, SignKind sign_kind = SignKind::SIGNED)
        : Type(kind), sign_kind_(sign_kind) {}
    
    /**
     * GetSignKind - Get the sign kind (signed or unsigned)
     */
    SignKind GetSignKind() const { return sign_kind_; }
    
    /**
     * IsSigned - Check if this is a signed type
     */
    bool IsSigned() const { return sign_kind_ == SignKind::SIGNED; }
    
    /**
     * IsUnsigned - Check if this is an unsigned type
     */
    bool IsUnsigned() const { return sign_kind_ == SignKind::UNSIGNED; }
    
    /**
     * IsEqual - Check if this type is equal to another type
     */
    bool IsEqual(const Type* other) const override;
    
    /**
     * ToString - Convert the type to a string representation
     */
    std::string ToString() const override;
    
private:
    SignKind sign_kind_;
};

/**
 * BoolType - Represents the bool type
 */
class BoolType : public PrimitiveType {
public:
    /**
     * Constructor
     */
    BoolType() : PrimitiveType(Kind::BOOL) {}
    
    /**
     * GetSize - Get the size of the type in bytes
     */
    size_t GetSize() const override { return 1; }
    
    /**
     * GetAlignment - Get the alignment of the type in bytes
     */
    size_t GetAlignment() const override { return 1; }
    
    /**
     * ToString - Convert the type to a string representation
     */
    std::string ToString() const override { return "bool"; }
};

/**
 * CharType - Represents the char type
 */
class CharType : public PrimitiveType {
public:
    /**
     * Constructor
     */
    CharType(SignKind sign_kind = SignKind::SIGNED)
        : PrimitiveType(Kind::CHAR, sign_kind) {}
    
    /**
     * GetSize - Get the size of the type in bytes
     */
    size_t GetSize() const override { return 1; }
    
    /**
     * GetAlignment - Get the alignment of the type in bytes
     */
    size_t GetAlignment() const override { return 1; }
    
    /**
     * ToString - Convert the type to a string representation
     */
    std::string ToString() const override;
};

/**
 * ShortType - Represents the short type
 */
class ShortType : public PrimitiveType {
public:
    /**
     * Constructor
     */
    ShortType(SignKind sign_kind = SignKind::SIGNED)
        : PrimitiveType(Kind::SHORT, sign_kind) {}
    
    /**
     * GetSize - Get the size of the type in bytes
     */
    size_t GetSize() const override { return 2; }
    
    /**
     * GetAlignment - Get the alignment of the type in bytes
     */
    size_t GetAlignment() const override { return 2; }
    
    /**
     * ToString - Convert the type to a string representation
     */
    std::string ToString() const override;
};

/**
 * IntType - Represents the int type
 */
class IntType : public PrimitiveType {
public:
    /**
     * Constructor
     */
    IntType(SignKind sign_kind = SignKind::SIGNED)
        : PrimitiveType(Kind::INT, sign_kind) {}
    
    /**
     * GetSize - Get the size of the type in bytes
     */
    size_t GetSize() const override { return 4; }
    
    /**
     * GetAlignment - Get the alignment of the type in bytes
     */
    size_t GetAlignment() const override { return 4; }
    
    /**
     * ToString - Convert the type to a string representation
     */
    std::string ToString() const override;
};

/**
 * LongType - Represents the long type (64-bit)
 */
class LongType : public PrimitiveType {
public:
    /**
     * Constructor
     */
    LongType(SignKind sign_kind = SignKind::SIGNED)
        : PrimitiveType(Kind::LONG, sign_kind) {}
    
    /**
     * GetSize - Get the size of the type in bytes
     */
    size_t GetSize() const override { return 8; }
    
    /**
     * GetAlignment - Get the alignment of the type in bytes
     */
    size_t GetAlignment() const override { return 8; }
    
    /**
     * ToString - Convert the type to a string representation
     */
    std::string ToString() const override;
};

/**
 * FloatType - Represents the float type
 */
class FloatType : public PrimitiveType {
public:
    /**
     * Constructor
     */
    FloatType() : PrimitiveType(Kind::FLOAT) {}
    
    /**
     * GetSize - Get the size of the type in bytes
     */
    size_t GetSize() const override { return 4; }
    
    /**
     * GetAlignment - Get the alignment of the type in bytes
     */
    size_t GetAlignment() const override { return 4; }
    
    /**
     * ToString - Convert the type to a string representation
     */
    std::string ToString() const override { return "float"; }
};

/**
 * DoubleType - Represents the double type
 */
class DoubleType : public PrimitiveType {
public:
    /**
     * Constructor
     */
    DoubleType() : PrimitiveType(Kind::DOUBLE) {}
    
    /**
     * GetSize - Get the size of the type in bytes
     */
    size_t GetSize() const override { return 8; }
    
    /**
     * GetAlignment - Get the alignment of the type in bytes
     */
    size_t GetAlignment() const override { return 8; }
    
    /**
     * ToString - Convert the type to a string representation
     */
    std::string ToString() const override { return "double"; }
};

/**
 * PointerType - Represents a pointer type
 */
class PointerType : public Type {
public:
    /**
     * Constructor
     */
    PointerType(std::shared_ptr<Type> pointee_type)
        : Type(Kind::POINTER), pointee_type_(pointee_type) {}
    
    /**
     * GetPointeeType - Get the pointee type
     */
    std::shared_ptr<Type> GetPointeeType() const { return pointee_type_; }
    
    /**
     * GetSize - Get the size of the type in bytes
     */
    size_t GetSize() const override { return 8; }  // 64-bit
    
    /**
     * GetAlignment - Get the alignment of the type in bytes
     */
    size_t GetAlignment() const override { return 8; }  // 64-bit
    
    /**
     * ToString - Convert the type to a string representation
     */
    std::string ToString() const override;
    
    /**
     * IsEqual - Check if this type is equal to another type
     */
    bool IsEqual(const Type* other) const override;
    
private:
    std::shared_ptr<Type> pointee_type_;
};

/**
 * ArrayType - Represents an array type
 */
class ArrayType : public Type {
public:
    /**
     * Constructor
     */
    ArrayType(std::shared_ptr<Type> element_type, size_t size)
        : Type(Kind::ARRAY), element_type_(element_type), size_(size) {}
    
    /**
     * GetElementType - Get the element type
     */
    std::shared_ptr<Type> GetElementType() const { return element_type_; }
    
    /**
     * GetSize - Get the size of the type in bytes
     */
    size_t GetSize() const override { return element_type_->GetSize() * size_; }
    
    /**
     * GetNumElements - Get the number of elements in the array
     */
    size_t GetNumElements() const { return size_; }
    
    /**
     * GetAlignment - Get the alignment of the type in bytes
     */
    size_t GetAlignment() const override { return element_type_->GetAlignment(); }
    
    /**
     * ToString - Convert the type to a string representation
     */
    std::string ToString() const override;
    
    /**
     * IsEqual - Check if this type is equal to another type
     */
    bool IsEqual(const Type* other) const override;
    
private:
    std::shared_ptr<Type> element_type_;
    size_t size_;
};

/**
 * StructType - Represents a struct type
 */
class StructType : public Type {
public:
    /**
     * Constructor
     */
    StructType(const std::string& name)
        : Type(Kind::STRUCT), name_(name), size_(0), alignment_(0), complete_(false) {}
    
    /**
     * GetName - Get the name of the struct
     */
    const std::string& GetName() const { return name_; }
    
    /**
     * AddField - Add a field to the struct
     */
    void AddField(const std::string& name, std::shared_ptr<Type> type);
    
    /**
     * GetFields - Get the fields of the struct
     */
    const std::vector<std::pair<std::string, std::shared_ptr<Type>>>& GetFields() const { return fields_; }
    
    /**
     * GetFieldOffsets - Get the field offsets
     */
    const std::vector<size_t>& GetFieldOffsets() const { return field_offsets_; }
    
    /**
     * GetFieldOffset - Get the offset of a field by name
     */
    size_t GetFieldOffset(const std::string& name) const;
    
    /**
     * GetFieldType - Get the type of a field by name
     */
    std::shared_ptr<Type> GetFieldType(const std::string& name) const;
    
    /**
     * GetSize - Get the size of the type in bytes
     */
    size_t GetSize() const override;
    
    /**
     * GetAlignment - Get the alignment of the type in bytes
     */
    size_t GetAlignment() const override;
    
    /**
     * IsComplete - Check if the struct is complete
     */
    bool IsComplete() const { return complete_; }
    
    /**
     * SetComplete - Set the struct as complete
     */
    void SetComplete();
    
    /**
     * ToString - Convert the type to a string representation
     */
    std::string ToString() const override;
    
    /**
     * IsEqual - Check if this type is equal to another type
     */
    bool IsEqual(const Type* other) const override;
    
private:
    std::string name_;
    std::vector<std::pair<std::string, std::shared_ptr<Type>>> fields_;
    std::vector<size_t> field_offsets_;
    mutable size_t size_;
    mutable size_t alignment_;
    bool complete_;
};

/**
 * EnumType - Represents an enum type
 */
class EnumType : public Type {
public:
    /**
     * Constructor
     */
    EnumType(const std::string& name, std::shared_ptr<Type> base_type)
        : Type(Kind::ENUM), name_(name), base_type_(base_type) {}
    
    /**
     * GetName - Get the name of the enum
     */
    const std::string& GetName() const { return name_; }
    
    /**
     * GetBaseType - Get the base type of the enum
     */
    std::shared_ptr<Type> GetBaseType() const { return base_type_; }
    
    /**
     * AddValue - Add a value to the enum
     */
    void AddValue(const std::string& name, int64_t value) {
        values_.push_back({name, value});
    }
    
    /**
     * GetValues - Get the values of the enum
     */
    const std::vector<std::pair<std::string, int64_t>>& GetValues() const { return values_; }
    
    /**
     * GetSize - Get the size of the type in bytes
     */
    size_t GetSize() const override { return base_type_->GetSize(); }
    
    /**
     * GetAlignment - Get the alignment of the type in bytes
     */
    size_t GetAlignment() const override { return base_type_->GetAlignment(); }
    
    /**
     * ToString - Convert the type to a string representation
     */
    std::string ToString() const override;
    
    /**
     * IsEqual - Check if this type is equal to another type
     */
    bool IsEqual(const Type* other) const override;
    
private:
    std::string name_;
    std::shared_ptr<Type> base_type_;
    std::vector<std::pair<std::string, int64_t>> values_;
};

/**
 * FunctionType - Represents a function type
 */
class FunctionType : public Type {
public:
    /**
     * Constructor
     */
    FunctionType(std::shared_ptr<Type> return_type,
                 std::vector<std::shared_ptr<Type>> param_types,
                 bool is_variadic = false)
        : Type(Kind::FUNCTION),
          return_type_(return_type),
          param_types_(param_types),
          is_variadic_(is_variadic) {}
    
    /**
     * GetReturnType - Get the return type
     */
    std::shared_ptr<Type> GetReturnType() const { return return_type_; }
    
    /**
     * GetParamTypes - Get the parameter types
     */
    const std::vector<std::shared_ptr<Type>>& GetParamTypes() const { return param_types_; }
    
    /**
     * IsVariadic - Check if this is a variadic function
     */
    bool IsVariadic() const { return is_variadic_; }
    
    /**
     * GetSize - Get the size of the type in bytes (0 for functions)
     */
    size_t GetSize() const override { return 0; }
    
    /**
     * GetAlignment - Get the alignment of the type in bytes (0 for functions)
     */
    size_t GetAlignment() const override { return 0; }
    
    /**
     * ToString - Convert the type to a string representation
     */
    std::string ToString() const override;
    
    /**
     * IsEqual - Check if this type is equal to another type
     */
    bool IsEqual(const Type* other) const override;
    
private:
    std::shared_ptr<Type> return_type_;
    std::vector<std::shared_ptr<Type>> param_types_;
    bool is_variadic_;
};

} // namespace dsLang

#endif // DSLANG_TYPE_H
