/*************************************************************************
> File Name       : MySqlTypeHack.h
> Brief           : MySql类型定义
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月11日  18时06分45秒
************************************************************************/
#pragma once

#include "mysql.h"

#include <type_traits>

using MySqlBool = std::remove_pointer_t<decltype(std::declval<MYSQL_BIND>().is_null)>;

using MySqlHandle = MYSQL;
using MySqlResult = MYSQL_RES;
using MySqlField  = MYSQL_FIELD;
using MySqlBind   = MYSQL_BIND;
using MySqlStmt   = MYSQL_STMT;

// clang-format off

// 基本类型与MySql数据类型映射
template <typename T>
struct MySqlType{};

template <>
struct MySqlType<uint8_t> : std::integral_constant<enum_field_types, MYSQL_TYPE_TINY>{};

template <>
struct MySqlType<uint16_t> : std::integral_constant<enum_field_types, MYSQL_TYPE_SHORT>{};

template <>
struct MySqlType<uint32_t> : std::integral_constant<enum_field_types, MYSQL_TYPE_LONG>{};

template <>
struct MySqlType<uint64_t> : std::integral_constant<enum_field_types, MYSQL_TYPE_LONGLONG>{};

template <>
struct MySqlType<int8_t> : std::integral_constant<enum_field_types, MYSQL_TYPE_TINY>{};

template <>
struct MySqlType<int16_t> : std::integral_constant<enum_field_types, MYSQL_TYPE_SHORT>{};

template <>
struct MySqlType<int32_t> : std::integral_constant<enum_field_types, MYSQL_TYPE_LONG>{};

template <>
struct MySqlType<int64_t> : std::integral_constant<enum_field_types, MYSQL_TYPE_LONGLONG>{};

template <>
struct MySqlType<float> : std::integral_constant<enum_field_types, MYSQL_TYPE_FLOAT>{};

template <>
struct MySqlType<double> : std::integral_constant<enum_field_types, MYSQL_TYPE_DOUBLE>{};

// clang-format on