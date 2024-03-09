rule("3rdPartyRule")
    on_load(function (target) 
        target:set("targetdir", target:targetdir().."/3rdParty")
    end)
rule_end()

target("asio")
    set_kind("static")
    add_rules("3rdPartyRule")
    set_group("3rdParty")
    add_headerfiles("./asio/asio/include/*.hpp", {prefixdir = "asio"})
    add_defines("ASIO_SEPARATE_COMPILATION")
    add_includedirs("asio/asio/include", {public = true})
    add_files("asio/asio/src/asio.cpp")
target_end()

target("spdlog")
    set_kind("static")
    add_rules("3rdPartyRule")
    set_group("3rdParty")
    add_headerfiles("spdlog/include/spdlog/**.h", {prefixdir="spdlog"})
    add_includedirs("spdlog/include", {public = true})
    add_defines("SPDLOG_COMPILED_LIB")
    add_files("spdlog/src/*.cpp")
target_end()

target("magic_enum")
    set_kind("headeronly")
    add_rules("3rdPartyRule")
    add_headerfiles("magic_enum/include/magic_enum/*.hpp", {prefixdir="magic_enum"})
    add_includedirs("magic_enum/include", {public = true})
target_end()

package("protobuf")
    add_deps("cmake", "zlib")
    set_policy("package.install_locally", true)
    -- set_policy("package.install_always", true)

    set_sourcedir(path.join(os.scriptdir(), "protobuf"))

    if is_plat("windows") then
        add_links("libprotobuf")
    else
        add_links("protobuf")
    end

    if is_plat("linux") then
        add_syslinks("pthread")
    end

    on_load(function (package)
        package:addenv("PATH", "bin")
        if package:config("zlib") then
            package:add("deps", "zlib")
        end
    end)

    on_install("windows", "linux", function (package)
        -- os.cd("cmake")
        -- io.replace("CMakeLists.txt", "set(protobuf_DEBUG_POSTFIX \"d\"", "set(protobuf_DEBUG_POSTFIX \"d\"", {plain = true})
        local configs = {"-Dprotobuf_BUILD_TESTS=OFF", "-Dprotobuf_BUILD_PROTOC_BINARIES=ON"}
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        if package:is_plat("windows") then
            table.insert(configs, "-Dprotobuf_MSVC_STATIC_RUNTIME=" .. (package:config("vs_runtime"):startswith("MT") and "ON" or "OFF"))
            if package:config("shared") then
                package:add("defines", "PROTOBUF_USE_DLLS")
            end
        end
        if package:config("zlib") then
            table.insert(configs, "-Dprotobuf_WITH_ZLIB=ON")
        end
        import("package.tools.cmake").install(package, configs, {buildir = "build"})
        os.trycp("build/Release/protoc.exe", package:installdir("bin"))
    end)