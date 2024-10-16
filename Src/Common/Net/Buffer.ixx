module;

#include <vector>
#include <cassert>
#include <string>
#include <cstring>
#include <algorithm>

export module Common:Net.Buffer;

export namespace Net
{
    class MessageBuffer final
    {
        using SizeType = std::vector<uint8_t>::size_type;

    public:
        static constexpr size_t INITIAL_BUFFER_SIZE = 1024;

        explicit MessageBuffer(size_t initialSize = INITIAL_BUFFER_SIZE) : _buffer(initialSize)
        {
        }

        MessageBuffer(const MessageBuffer &buffer) = default;

        MessageBuffer &operator=(MessageBuffer buffer)
        {
            swap(buffer);
            return *this;
        }

        MessageBuffer(MessageBuffer &&buffer) noexcept
            : _readIndex(buffer._readIndex)
            , _writeIndex(buffer._writeIndex)
            , _buffer(std::move(buffer._buffer))
        {
            buffer._readIndex  = 0;
            buffer._writeIndex = 0;
        }

        MessageBuffer &operator=(MessageBuffer &&buffer) noexcept
        {
            buffer.swap(*this);
            return *this;
        }

        ~MessageBuffer() = default;

        void swap(MessageBuffer &buffer) noexcept
        {
            std::swap(_readIndex, buffer._readIndex);
            std::swap(_writeIndex, buffer._writeIndex);
            _buffer.swap(buffer._buffer);
        }

        uint8_t *GetBasePointer()
        {
            return _buffer.data();
        }

        uint8_t *GetReadPointer()
        {
            return GetBasePointer() + _readIndex;
        }

        uint8_t *GetWritPointer()
        {
            return GetBasePointer() + _writeIndex;
        }

        void ReadDone(size_t len)
        {
            _readIndex += len;
        }

        void WriteDone(size_t len)
        {
            _writeIndex += len;
        }

        [[nodiscard]] size_t ReadableBytes() const
        {
            return _writeIndex - _readIndex;
        }

        [[nodiscard]] size_t WritableBytes() const
        {
            return _buffer.size() - _writeIndex;
        }

        void EnsureWritableBytes(size_t len)
        {
            if (WritableBytes() < len)
            {
                MakeSpace(len);
            }

            assert(WritableBytes() >= 0);
        }

        void EnsureFreeSpace()
        {
            MakeSpace(_buffer.size() * 3 / 2);
        }

        template <typename PODType>
            requires std::is_standard_layout_v<PODType> && std::is_trivial_v<PODType>
        void Write(const PODType &data)
        {
            EnsureWritableBytes(sizeof(data));
            std::memcpy(GetWritPointer(), &data, sizeof(data));
            _writeIndex += sizeof(data);
        }

        template <typename PODType>
            requires std::is_standard_layout_v<PODType> && std::is_trivial_v<PODType>
        friend MessageBuffer &operator<<(MessageBuffer &buffer, const PODType &data)
        {
            buffer.Write(data);
            return buffer;
        }

        void Write(std::string_view str)
        {
            Write(str.data(), str.size());
        }

        void Write(const uint8_t *data, size_t len)
        {
            EnsureWritableBytes(len);
            std::copy(data, data + len, GetWritPointer());
            _writeIndex += len;
        }

        void Write(const void *data, size_t len)
        {
            const uint8_t *byteData = static_cast<const uint8_t *>(data);
            Write(byteData, len);
        }

        template <typename PODType>
            requires std::is_standard_layout_v<PODType> && std::is_trivial_v<PODType>
        PODType Read()
        {
            PODType result;
            std::memcpy(&result, GetReadPointer(), sizeof(result));
            ConsumeBytes(sizeof(PODType));
            return result;
        }

        size_t Read(void *data, size_t len)
        {
            assert(len <= ReadableBytes());
            auto canReadLen = (std::min)(len, ReadableBytes());
            std::memcpy(data, GetReadPointer(), canReadLen);
            ConsumeBytes(canReadLen);
            return canReadLen;
        }

        template <typename PODType>
            requires std::is_standard_layout_v<PODType> && std::is_trivial_v<PODType>
        friend MessageBuffer &operator>>(MessageBuffer &buffer, PODType &data)
        {
            data = buffer.Read<PODType>();
            return buffer;
        }

        std::string ReadAsString(size_t len)
        {
            assert(len <= ReadableBytes());
            std::string result(reinterpret_cast<const char *>(GetReadPointer()), len);
            ConsumeBytes(len);
            return result;
        }

        std::string ReadAllAsString()
        {
            return ReadAsString(ReadableBytes());
        }

        void Shrink(size_t reserve)
        {
            MessageBuffer other;
            other.EnsureWritableBytes(ReadableBytes() + reserve);
            other.Write(GetReadPointer(), ReadableBytes());
            swap(other);
        }

    private:
        void ConsumeBytes(size_t len)
        {
            assert(len <= ReadableBytes());
            if (len < ReadableBytes())
            {
                _readIndex += len;
            }
            else
            {
                ResetBuffer();
            }
        }

        void ResetBuffer()
        {
            _readIndex  = 0;
            _writeIndex = 0;
        }

        void MakeSpace(size_t len)
        {
            if (WritableBytes() < len)
            {
                _buffer.resize(_writeIndex + len);
            }
            else
            {
                const size_t readableBytes = ReadableBytes();
                std::copy(GetReadPointer(), GetWritPointer(), _buffer.data());
                _readIndex  = 0;
                _writeIndex = _readIndex + readableBytes;
                assert(readableBytes == ReadableBytes());
            }
        }

        SizeType             _readIndex {};
        SizeType             _writeIndex {};
        std::vector<uint8_t> _buffer;
    };

    inline void swap(MessageBuffer &lhs, MessageBuffer &rhs) noexcept
    {
        lhs.swap(rhs);
    }
} // namespace Net
