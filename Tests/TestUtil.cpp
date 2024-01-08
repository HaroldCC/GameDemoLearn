#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

#include "Common/Util/Util.h"

using namespace Util;

TEST_CASE("StringEqual function test")
{
    REQUIRE(StringEqual("Hello", "hello"));
    REQUIRE(StringEqual("WORLD", "world"));
    REQUIRE_FALSE(StringEqual("Hello", "world"));
}

TEST_CASE("FromString function test")
{
    REQUIRE(detail::For<int>::FromString("123") == 123);
    REQUIRE(detail::For<int>::FromString("0xFF", 16) == 255);
    REQUIRE(detail::For<int>::FromString("0b1010", 2) == 10);

    REQUIRE_FALSE(detail::For<int>::FromString("abc").has_value());
    REQUIRE_FALSE(detail::For<int>::FromString("").has_value());
    REQUIRE_FALSE(detail::For<int>::FromString("0b123", 2).has_value());
}

TEST_CASE("FromBool function test")
{
    REQUIRE(detail::ForBool::FromString("1") == true);
    REQUIRE(detail::ForBool::FromString("0") == false);
    REQUIRE(detail::ForBool::FromString("y") == true);
    REQUIRE(detail::ForBool::FromString("n") == false);
    REQUIRE(detail::ForBool::FromString("yes") == true);
    REQUIRE(detail::ForBool::FromString("no") == false);

    REQUIRE_FALSE(detail::ForBool::FromString("2").has_value());
    REQUIRE_FALSE(detail::ForBool::FromString("").has_value());
    REQUIRE_FALSE(detail::ForBool::FromString("true").has_value());
}

TEST_CASE("FromFloat function test")
{
    REQUIRE(detail::ForFloat<float>::FromString("123.45").value() == doctest::Approx(123.45f));
    REQUIRE(detail::ForFloat<double>::FromString("1e-10").value() == doctest::Approx(1e-10));
    REQUIRE(detail::ForFloat<double>::FromString("0x1.8p1", 16).value() == doctest::Approx(3.0));

    REQUIRE_FALSE(detail::ForFloat<float>::FromString("abc").has_value());
    REQUIRE_FALSE(detail::ForFloat<double>::FromString("").has_value());
    REQUIRE_FALSE(detail::ForFloat<double>::FromString("0x1.2p3", 10).has_value());
}

TEST_CASE("StringTo function test")
{
    REQUIRE(StringTo<int>("123") == 123);
    REQUIRE(StringTo<float>("1.23") == doctest::Approx(1.23f));
    REQUIRE(StringTo<double>("1e-10") == doctest::Approx(1e-10));
    REQUIRE(StringTo<bool>("1") == true);
    REQUIRE(StringTo<bool>("yes") == true);
    REQUIRE(StringTo<double>("0x1.2p3").has_value() == true);

    REQUIRE_FALSE(StringTo<int>("abc").has_value());
    REQUIRE_FALSE(StringTo<float>("").has_value());
    REQUIRE_FALSE(StringTo<bool>("true").has_value());
}

TEST_CASE("ToString function test")
{
    REQUIRE(ToString(123) == "123");
    REQUIRE(ToString(1.23f) == "1.23");
    REQUIRE(ToString(1e-10) == "1e-10");
    REQUIRE(ToString(true) == "1");

    REQUIRE(ToString(123) != "abc");
    REQUIRE(ToString(1.23f) != "");
    REQUIRE(ToString(1e-10) != "0x1.2p3");
    REQUIRE(ToString(true) != "false");
}