/*************************************************************************
> File Name       : Field.h
> Brief           : 数据库字段
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月12日  15时20分48秒
************************************************************************/
#pragma once

#include <cstdint>
#include <string_view>
#include <optional>
#include <string>

namespace Database
{
    enum class DatabaseFieldType : uint8_t
    {
        Null,
        UInt8,
        Int8,
        UInt16,
        Int16,
        UInt32,
        Int32,
        UInt64,
        Int64,
        Float,
        Double,
        Decimal,
        Date,
        Binary
    };

    // 查询结果字段信息
    struct QueryResultFieldMetadata
    {
        std::string_view  tableName;
        std::string_view  tableAlias;
        std::string_view  name;
        std::string_view  alias;
        std::string_view  typeName;
        uint32_t          index     = 0;
        DatabaseFieldType fieldType = DatabaseFieldType::Null;
        bool              bBinary   = false;
    };

    class Field final
    {
        friend class QueryResultSet;
        friend class PreparedQueryResultSet;

    private:
        [[nodiscard]] bool GetBool() const
        {
            return GetUInt8() == 1;
        }

        [[nodiscard]] uint8_t     GetUInt8() const;
        [[nodiscard]] int8_t      GetInt8() const;
        [[nodiscard]] uint16_t    GetUInt16() const;
        [[nodiscard]] int16_t     GetInt16() const;
        [[nodiscard]] uint32_t    GetUInt32() const;
        [[nodiscard]] int32_t     GetInt32() const;
        [[nodiscard]] uint64_t    GetUInt64() const;
        [[nodiscard]] int64_t     GetInt64() const;
        [[nodiscard]] float       GetFloat() const;
        [[nodiscard]] double      GetDouble() const;
        [[nodiscard]] const char *GetCString() const;

        [[nodiscard]] bool IsNull() const
        {
            return _pValue == nullptr;
        }

        template <typename T>
        constexpr std::optional<T> CheckType() const;

        void LogWrongGetter(std::string_view strGetter) const;

        void SetValue(const char *newValue, uint32_t length);

        void SetMetadata(const QueryResultFieldMetadata *fieldMeta);

    public:
        operator uint8_t()
        {
            return GetUInt8();
        }

        operator uint16_t()
        {
            return GetUInt16();
        }

        operator uint32_t()
        {
            return GetUInt32();
        }

        operator uint64_t()
        {
            return GetUInt64();
        }

        operator int8_t()
        {
            return GetInt8();
        }

        operator int16_t()
        {
            return GetInt16();
        }

        operator int32_t()
        {
            return GetInt32();
        }

        operator int64_t()
        {
            return GetInt64();
        }

        operator const char *()
        {
            return GetCString();
        }

        operator std::string()
        {
            const char *str = GetCString();
            return str == nullptr ? std::string {} : std::string {str};
        }

    private:
        const char                     *_pValue {nullptr};
        uint32_t                        _length {0};
        const QueryResultFieldMetadata *_meta {nullptr};
    };

} // namespace Database