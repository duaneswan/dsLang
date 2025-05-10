/**
 * type.cpp - Type System Implementation for dsLang
 * 
 * This file implements the Type class hierarchy used to represent the types
 * of expressions and declarations in dsLang.
 */

#include "type.h"
#include <algorithm>
#include <sstream>

namespace dsLang {

/**
 * IsEqual - Check if this type is equal to another type
 */
bool Type::IsEqual(const Type* other) const {
    return other && kind_ == other->kind_;
}

/**
 * IsIntegral - Check if this is an integral type (bool, char, short, int, long, enum)
 */
bool Type::IsIntegral() const {
    return IsBool() || IsChar() || IsShort() || IsInt() || IsLong() || IsEnum();
}

/**
 * IsFloatingPoint - Check if this is a floating-point type (float, double)
 */
bool Type::IsFloatingPoint() const {
    return IsFloat() || IsDouble();
}

/**
 * IsArithmetic - Check if this is an arithmetic type (integral or floating-point)
 */
bool Type::IsArithmetic() const {
    return IsIntegral() || IsFloatingPoint();
}

/**
 * IsScalar - Check if this is a scalar type (arithmetic, pointer, enum)
 */
bool Type::IsScalar() const {
    return IsArithmetic() || IsPointer() || IsEnum();
}

/**
 * IsEqual - Check if this primitive type is equal to another type
 */
bool PrimitiveType::IsEqual(const Type* other) const {
    if (!Type::IsEqual(other)) {
        return false;
    }
    
    const PrimitiveType* other_primitive = static_cast<const PrimitiveType*>(other);
    return sign_kind_ == other_primitive->sign_kind_;
}

/**
 * ToString - Convert the primitive type to a string representation
 */
std::string PrimitiveType::ToString() const {
    std::string result;
    
    if (IsUnsigned()) {
        result += "unsigned ";
    }
    
    switch (GetKind()) {
        case Kind::BOOL:
            result += "bool";
            break;
        case Kind::CHAR:
            result += "char";
            break;
        case Kind::SHORT:
            result += "short";
            break;
        case Kind::INT:
            result += "int";
            break;
        case Kind::LONG:
            result += "long";
            break;
        case Kind::FLOAT:
            result += "float";
            break;
        case Kind::DOUBLE:
            result += "double";
            break;
        default:
            result += "unknown";
            break;
    }
    
    return result;
}

/**
 * ToString - Convert the char type to a string representation
 */
std::string CharType::ToString() const {
    if (IsUnsigned()) {
        return "unsigned char";
    } else {
        return "char";
    }
}

/**
 * ToString - Convert the short type to a string representation
 */
std::string ShortType::ToString() const {
    if (IsUnsigned()) {
        return "unsigned short";
    } else {
        return "short";
    }
}

/**
 * ToString - Convert the int type to a string representation
 */
std::string IntType::ToString() const {
    if (IsUnsigned()) {
        return "unsigned int";
    } else {
        return "int";
    }
}

/**
 * ToString - Convert the long type to a string representation
 */
std::string LongType::ToString() const {
    if (IsUnsigned()) {
        return "unsigned long";
    } else {
        return "long";
    }
}

/**
 * ToString - Convert the pointer type to a string representation
 */
std::string PointerType::ToString() const {
    return pointee_type_->ToString() + "*";
}

/**
 * IsEqual - Check if this pointer type is equal to another type
 */
bool PointerType::IsEqual(const Type* other) const {
    if (!Type::IsEqual(other)) {
        return false;
    }
    
    const PointerType* other_ptr = static_cast<const PointerType*>(other);
    return pointee_type_->IsEqual(other_ptr->pointee_type_.get());
}

/**
 * ToString - Convert the array type to a string representation
 */
std::string ArrayType::ToString() const {
    std::ostringstream oss;
    if (has_constant_size_) {
        oss << element_type_->ToString() << "[" << size_ << "]";
    } else {
        oss << element_type_->ToString() << "[]"; // Size is not known yet
    }
    return oss.str();
}

/**
 * IsEqual - Check if this array type is equal to another type
 */
bool ArrayType::IsEqual(const Type* other) const {
    if (!Type::IsEqual(other)) {
        return false;
    }
    
    const ArrayType* other_array = static_cast<const ArrayType*>(other);
    
    // If both arrays have constant size, compare the sizes
    if (has_constant_size_ && other_array->has_constant_size_) {
        return size_ == other_array->size_ && 
               element_type_->IsEqual(other_array->element_type_.get());
    }
    
    // If one has constant size and the other doesn't, they're not equal
    if (has_constant_size_ != other_array->has_constant_size_) {
        return false;
    }
    
    // Both don't have constant size, just compare element types
    return element_type_->IsEqual(other_array->element_type_.get());
}

/**
 * AddField - Add a field to the struct
 */
void StructType::AddField(const std::string& name, std::shared_ptr<Type> type) {
    // Cannot add fields to a completed struct
    if (complete_) {
        return;
    }
    
    fields_.push_back({name, type});
}

/**
 * GetFieldOffset - Get the offset of a field by name
 */
size_t StructType::GetFieldOffset(const std::string& name) const {
    // Ensure the struct is complete
    if (!complete_) {
        return 0;
    }
    
    for (size_t i = 0; i < fields_.size(); ++i) {
        if (fields_[i].first == name) {
            return field_offsets_[i];
        }
    }
    
    return 0;  // Field not found
}

/**
 * GetFieldType - Get the type of a field by name
 */
std::shared_ptr<Type> StructType::GetFieldType(const std::string& name) const {
    for (const auto& field : fields_) {
        if (field.first == name) {
            return field.second;
        }
    }
    
    return nullptr;  // Field not found
}

/**
 * GetSize - Get the size of the struct type in bytes
 */
size_t StructType::GetSize() const {
    if (!complete_) {
        return 0;
    }
    
    return size_;
}

/**
 * GetAlignment - Get the alignment of the struct type in bytes
 */
size_t StructType::GetAlignment() const {
    if (!complete_) {
        return 0;
    }
    
    return alignment_;
}

/**
 * SetComplete - Set the struct as complete
 */
void StructType::SetComplete() {
    if (complete_) {
        return;
    }
    
    complete_ = true;
    
    // Calculate field offsets and struct size
    size_t current_offset = 0;
    
    for (const auto& field : fields_) {
        size_t field_align = field.second->GetAlignment();
        
        // Adjust the current offset to meet the field's alignment requirement
        current_offset = (current_offset + field_align - 1) / field_align * field_align;
        
        field_offsets_.push_back(current_offset);
        
        // Update the struct alignment (the maximum of all field alignments)
        alignment_ = std::max(alignment_, field_align);
        
        // Move to the next field
        current_offset += field.second->GetSize();
    }
    
    // Round up the final size to be a multiple of the alignment
    size_ = (current_offset + alignment_ - 1) / alignment_ * alignment_;
}

/**
 * ToString - Convert the struct type to a string representation
 */
std::string StructType::ToString() const {
    return "struct " + name_;
}

/**
 * IsEqual - Check if this struct type is equal to another type
 */
bool StructType::IsEqual(const Type* other) const {
    if (!Type::IsEqual(other)) {
        return false;
    }
    
    const StructType* other_struct = static_cast<const StructType*>(other);
    
    // If both are incomplete, compare names
    if (!complete_ && !other_struct->complete_) {
        return name_ == other_struct->name_;
    }
    
    // If one is complete and the other is incomplete, they're not equal
    if (complete_ != other_struct->complete_) {
        return false;
    }
    
    // Both are complete, compare fields
    if (fields_.size() != other_struct->fields_.size()) {
        return false;
    }
    
    for (size_t i = 0; i < fields_.size(); ++i) {
        if (fields_[i].first != other_struct->fields_[i].first ||
            !fields_[i].second->IsEqual(other_struct->fields_[i].second.get())) {
            return false;
        }
    }
    
    return true;
}

/**
 * ToString - Convert the enum type to a string representation
 */
std::string EnumType::ToString() const {
    return "enum " + name_;
}

/**
 * IsEqual - Check if this enum type is equal to another type
 */
bool EnumType::IsEqual(const Type* other) const {
    if (!Type::IsEqual(other)) {
        return false;
    }
    
    const EnumType* other_enum = static_cast<const EnumType*>(other);
    
    // Compare names
    if (name_ != other_enum->name_) {
        return false;
    }
    
    // Compare base types
    if (!base_type_->IsEqual(other_enum->base_type_.get())) {
        return false;
    }
    
    // Compare values
    if (values_.size() != other_enum->values_.size()) {
        return false;
    }
    
    for (size_t i = 0; i < values_.size(); ++i) {
        if (values_[i].first != other_enum->values_[i].first ||
            values_[i].second != other_enum->values_[i].second) {
            return false;
        }
    }
    
    return true;
}

/**
 * ToString - Convert the function type to a string representation
 */
std::string FunctionType::ToString() const {
    std::ostringstream oss;
    oss << return_type_->ToString() << "(";
    
    for (size_t i = 0; i < param_types_.size(); ++i) {
        if (i > 0) {
            oss << ", ";
        }
        oss << param_types_[i]->ToString();
    }
    
    if (is_variadic_) {
        if (!param_types_.empty()) {
            oss << ", ";
        }
        oss << "...";
    }
    
    oss << ")";
    return oss.str();
}

/**
 * IsEqual - Check if this function type is equal to another type
 */
bool FunctionType::IsEqual(const Type* other) const {
    if (!Type::IsEqual(other)) {
        return false;
    }
    
    const FunctionType* other_func = static_cast<const FunctionType*>(other);
    
    // Compare return types
    if (!return_type_->IsEqual(other_func->return_type_.get())) {
        return false;
    }
    
    // Compare variadic flag
    if (is_variadic_ != other_func->is_variadic_) {
        return false;
    }
    
    // Compare parameter types
    if (param_types_.size() != other_func->param_types_.size()) {
        return false;
    }
    
    for (size_t i = 0; i < param_types_.size(); ++i) {
        if (!param_types_[i]->IsEqual(other_func->param_types_[i].get())) {
            return false;
        }
    }
    
    return true;
}

} // namespace dsLang
