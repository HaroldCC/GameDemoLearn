add_requires("doctest")

rule("TestRule")
    on_load(function (target) 
        target:add("deps", "Common")
        target:add("packages", "doctest")
        target:set("targetdir", target:targetdir().."/Tests")
    end)
rule_end()

target("TestLog")
    set_kind("binary")
    add_rules("TestRule", "CommonRule")
    add_files("TestLog.cpp")

target("TestUtil")
    set_kind("binary")
    add_rules("TestRule", "CommonRule")
    add_files("TestUtil.cpp")

target("TestTimeUtil")
    set_kind("binary")
    add_rules("TestRule", "CommonRule")
    add_files("TestTimeUtil.cpp")

target("TestBuffer")
    set_kind("binary")
    add_rules("TestRule", "CommonRule")
    add_files("TestBuffer.cpp")

target("TestCoroutine")
    set_kind("binary")
    add_rules("CommonRule", "TestRule")
    add_files("Testcoroutine.cpp")
    add_includedirs("$(projectdir)/3rdParty/async_simple")

target("TestClient")
    set_kind("binary")
    add_rules("CommonRule", "TestRule")
    add_files("testClient.cpp")
    add_includedirs("$(projectdir)/3rdParty/async_simple")

includes("TestAngelScript")