#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "Common/Net/Buffer.h"

using Net::MessageBuffer;

// 测试 MessageBuffer 的构造函数和基本方法
TEST_CASE("MessageBuffer - Constructor and Basic Methods")
{
    MessageBuffer buffer;

    // 测试初始状态
    CHECK(buffer.ReadableBytes() == 0);
    CHECK(buffer.WritableBytes() == MessageBuffer::INITIAL_BUFFER_SIZE);

    // 测试写入后的状态
    uint8_t data[] = {0, 1, 2, 3, 4};
    buffer.Write(data, sizeof(data));
    CHECK(buffer.ReadableBytes() == sizeof(data));
    CHECK(buffer.WritableBytes() == MessageBuffer::INITIAL_BUFFER_SIZE - sizeof(data));

    // 测试读取后的状态
    uint8_t output[sizeof(data)];
    buffer.Read(output, sizeof(output));
    CHECK(buffer.ReadableBytes() == 0);

    // 检查读取的数据是否与写入的数据相同
    for (size_t i = 0; i < sizeof(data); ++i)
    {
        CHECK(data[i] == output[i]);
    }
}

// 测试字符串操作
TEST_CASE("MessageBuffer - String Operations")
{
    MessageBuffer buffer;
    std::string   testString = "Hello, World!";

    buffer.Write(testString);

    CHECK(buffer.ReadableBytes() == testString.size());

    std::string result = buffer.ReadAllAsString();
    CHECK(result == testString);
}

// 测试 POD 类型的读写
TEST_CASE("MessageBuffer - POD Type Read and Write")
{
    MessageBuffer buffer;
    int           testValue = 42;

    buffer << testValue; // 使用流运算符写入

    int outputValue = 0;
    buffer >> outputValue; // 使用流运算符读取

    CHECK(testValue == outputValue); // 检查写入和读取的值是否相同
}

TEST_CASE("Test initializing MessageBuffer")
{
    MessageBuffer buffer;

    CHECK(buffer.ReadableBytes() == 0);
    CHECK(buffer.WritableBytes() == MessageBuffer::INITIAL_BUFFER_SIZE);
}

TEST_CASE("Test writing to buffer and reading from buffer")
{
    MessageBuffer buffer;

    SUBCASE("Write and read pod type using operator<< and >>")
    {
        int data = 123456;
        buffer << data;

        int result;
        buffer >> result;

        CHECK(result == data);
    }

    SUBCASE("Write and read byte data directly")
    {
        const char *data = "test data";
        buffer.Write(reinterpret_cast<const uint8_t *>(data), strlen(data));
        CHECK(buffer.ReadableBytes() == strlen(data));

        char output[10];
        buffer.Read(output, strlen(data));
        output[strlen(data)] = '\0'; // Null terminate the string

        CHECK(strcmp(output, data) == 0);
    }

    SUBCASE("Write string and read back")
    {
        std::string data = "test string";
        buffer.Write(data);

        CHECK(buffer.ReadableBytes() == data.size());

        std::string output = buffer.ReadAllAsString();
        CHECK(output == data);
    }

    SUBCASE("Test buffer expansion")
    {
        std::vector<uint8_t> largeData(MessageBuffer::INITIAL_BUFFER_SIZE * 2);
        buffer.Write(largeData.data(), largeData.size());
        CHECK(buffer.ReadableBytes() == largeData.size());
    }
}

TEST_CASE("Check buffer adjustment after read and write")
{
    MessageBuffer buffer;

    SUBCASE("Check read/write indices after operations")
    {
        std::string data = "some data";
        buffer.Write(data);
        CHECK(buffer.GetReadPointer() == buffer.GetBasePointer());
        CHECK(buffer.GetWritPointer() != buffer.GetBasePointer());

        buffer.ReadAsString(data.size());
        CHECK(buffer.GetReadPointer() == buffer.GetWritPointer());
    }
}

TEST_CASE("测试MessageBuffer的初始化和基本操作")
{
    MessageBuffer mb;

    // 测试是否初始缓冲区为空
    CHECK(mb.ReadableBytes() == 0);
    CHECK(mb.WritableBytes() == MessageBuffer::INITIAL_BUFFER_SIZE);

    // 测试写入并确认读取的字节数
    const char *data = "Hello World";
    mb.Write(reinterpret_cast<const uint8_t *>(data), 11);
    CHECK(mb.ReadableBytes() == 11);
    CHECK(mb.WritableBytes() == MessageBuffer::INITIAL_BUFFER_SIZE - 11);

    // 测试读取
    auto   buffer     = std::make_unique<uint8_t[]>(11);
    size_t bytes_read = mb.Read(buffer.get(), 11);
    CHECK(bytes_read == 11);
    CHECK(std::strncmp(reinterpret_cast<char *>(buffer.get()), data, 11) == 0);
}

TEST_CASE("测试MessageBuffer的EnsureWritableBytes和MakeSpace逻辑")
{
    MessageBuffer mb(10); // 故意设置小的初始大小

    // 写入直到需要扩容
    const char *data = "1234567890"; // 10个字节
    mb.Write(reinterpret_cast<const uint8_t *>(data), 10);
    CHECK(mb.ReadableBytes() == 10);
    CHECK(mb.WritableBytes() == 0); // 应该没有可写空间了

    // 确保有可写空间
    mb.EnsureWritableBytes(5);      // 请求更多空间，触发扩容
    CHECK(mb.WritableBytes() >= 5); // 现在应该有更多可写空间了
}

TEST_CASE("测试MessageBuffer流操作符")
{
    MessageBuffer mb;
    uint32_t      testValue = 0x12345678;
    mb << testValue; // 流式写入
    uint32_t receivedValue;
    mb >> receivedValue; // 流式读取
    CHECK(testValue == receivedValue);
}

TEST_CASE("测试ReadAsString和ReadAllAsString")
{
    MessageBuffer mb;

    const std::string message = "A quick brown fox jumps over the lazy dog";
    mb.Write(reinterpret_cast<const uint8_t *>(message.c_str()), message.length());

    // 部分地读取字符串
    std::string partialMessage = mb.ReadAsString(17); // 读取"A quick brown fox"
    CHECK(partialMessage == "A quick brown fox");

    // 读取剩下的全部字符串
    std::string remainingMessage = mb.ReadAllAsString();
    CHECK(remainingMessage == " jumps over the lazy dog");
}