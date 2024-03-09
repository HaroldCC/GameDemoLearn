/*************************************************************************
> File Name       : PreparedStatement.h
> Brief           : 预处理语句
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2024年01月11日  17时15分35秒
************************************************************************/
#pragma once
#include "Common/Util/Assert.h"
#include "DatabaseEnv.h"

#include <variant>
#include <string>
#include <vector>
#include <format>

namespace Database
{
    struct PreparedStatementData
    {
        std::variant<bool,
                     uint8_t,
                     uint16_t,
                     uint32_t,
                     uint64_t,
                     int8_t,
                     int16_t,
                     int32_t,
                     int64_t,
                     float,
                     double,
                     std::string,
                     std::vector<uint8_t>,
                     std::nullptr_t>
            data;

        template <typename T>
            requires std::is_standard_layout_v<T> && std::is_trivial_v<T>
        constexpr static std::string ToString(T &&value)
        {
            return std::format("{}", std::forward<T>(value));
        }

        constexpr static std::string ToString(const std::vector<uint8_t> & /*unused*/)
        {
            return "BINARY";
        }

        constexpr static std::string ToString(std::nullptr_t)
        {
            return "NULL";
        }

        constexpr static std::string ToString(bool value)
        {
            return ToString<uint8_t>(value);
        }

        constexpr static std::string ToString(const std::string &value)
        {
            return value;
        }
    };

    class PreparedStatementBase
    {
    public:
        PreparedStatementBase(uint32_t preparedStatementIndex, uint8_t dataCapacity)
            : _preparedStatementIndex(preparedStatementIndex)
            , _statementData(dataCapacity)
        {
        }

        ~PreparedStatementBase() = default;

        PreparedStatementBase(const PreparedStatementBase &)            = delete;
        PreparedStatementBase(PreparedStatementBase &&)                 = delete;
        PreparedStatementBase &operator=(const PreparedStatementBase &) = delete;
        PreparedStatementBase &operator=(PreparedStatementBase &&)      = delete;

        template <typename T>
        void SerialValue(const SqlStmtData &sqlStmt, T &&value)
        {
            for (size_t i = 0; i < sqlStmt.argtypes.size(); ++i)
            {
                SetValue(static_cast<uint8_t>(i), sqlStmt.argtypes[i], std::forward<T>(value));
            }
        }

        template <typename ValueType>
        constexpr void SetValue(const uint8_t &index, SqlArgType type, ValueType &&value)
        {
            Assert(index < _statementData.size());

            using DecayType = std::decay_t<ValueType>;

            if constexpr (std::is_same_v<DecayType, uint8_t>)
            {
                _statementData[index].data.emplace<uint8_t>(std::forward<ValueType>(value));
            }
            else if constexpr (std::is_same_v<DecayType, uint16_t>)
            {
                _statementData[index].data.emplace<uint16_t>(std::forward<ValueType>(value));
            }
            else if constexpr (std::is_same_v<DecayType, uint32_t>)
            {
                _statementData[index].data.emplace<uint32_t>(std::forward<ValueType>(value));
            }
            else if constexpr (std::is_same_v<DecayType, uint64_t>)
            {
                _statementData[index].data.emplace<uint64_t>(std::forward<ValueType>(value));
            }
            else if constexpr (std::is_same_v<DecayType, int8_t>)
            {
                _statementData[index].data.emplace<int8_t>(std::forward<ValueType>(value));
            }
            else if constexpr (std::is_same_v<DecayType, int16_t>)
            {
                _statementData[index].data.emplace<int16_t>(std::forward<ValueType>(value));
            }
            else if constexpr (std::is_same_v<DecayType, int32_t>)
            {
                _statementData[index].data.emplace<int32_t>(std::forward<ValueType>(value));
            }
            else if constexpr (std::is_same_v<DecayType, int64_t>)
            {
                _statementData[index].data.emplace<int64_t>(std::forward<ValueType>(value));
            }
            else if constexpr (std::is_same_v<DecayType, float>)
            {
                _statementData[index].data.emplace<float>(std::forward<ValueType>(value));
            }
            else if constexpr (std::is_same_v<DecayType, double>)
            {
                _statementData[index].data.emplace<double>(std::forward<ValueType>(value));
            }
            else if constexpr (std::is_convertible_v<DecayType, std::string>)
            {
                _statementData[index].data.emplace<std::string>(std::forward<ValueType>(value));
            }
        }

        [[nodiscard]] uint32_t GetIndex() const

        {
            return _preparedStatementIndex;
        }

        [[nodiscard]] const std::vector<PreparedStatementData> &GetParameters() const
        {
            return _statementData;
        }

    private:
        uint32_t                           _preparedStatementIndex; // 预处理语句索引
        std::vector<PreparedStatementData> _statementData;
    };
} // namespace Database