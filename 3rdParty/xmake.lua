rule("3rdPartyRule")
    on_load(function (target) 
        target:set("targetdir", target:targetdir().."/3rdParty")
    end)
rule_end()

target("angelscript") do
	set_kind("static")

    add_rules("3rdPartyRule")
    set_group("3rdParty")

	add_headerfiles("angelscript/include/angelscript.h")
	add_headerfiles("angelscript/source/*.h", "angelscript/add_on/**.h")
	add_files("angelscript/source/*.cpp", "angelscript/source/as_callfunc_x64_msvc_asm.asm", "angelscript/add_on/**.cpp")

    add_includedirs("angelscript/include", "angelscript/add_on/", {public=true})
end

target("asbind") do
    set_kind("static")

    add_rules("3rdPartyRule")
    set_group("3rdParty")

    add_headerfiles("asbind20/include/**.hpp")
    add_files("asbind20/src/**.cpp")

    add_includedirs("asbind20/include", {public=true})
    add_deps("angelscript")
end

target("asio")
    set_kind("static")
    add_rules("3rdPartyRule")
    set_group("3rdParty")
    add_headerfiles("./asio/asio/include/*.hpp", {prefixdir = "asio"})
    add_defines("ASIO_SEPARATE_COMPILATION")
    add_includedirs("asio/asio/include", {public = true})
    add_files("asio/asio/src/asio.cpp")

    before_build(function (target) 
        if target:is_plat("windows") then
            target:add("defines", "_WIN32_WINDOWS", "_WINSOCK_DEPRECATED_NO_WARNINGS")
        end
    end)
target_end()

package("spdlog")
    add_deps("cmake")
    set_policy("package.install_locally", true)
    set_sourcedir(path.join(os.scriptdir(), "spdlog"))

    if is_plat("windows") then
        add_configs("shared", {description = "Build shared library.", default = false, type = "boolean", readonly = true})
        add_configs("wchar",  {description = "Support wchar api.", default = false, type = "boolean"})
    elseif is_plat("linux", "bsd") then
        add_syslinks("pthread")
    end

    add_configs("header_only",     {description = "Use header only version.", default = true, type = "boolean"})
    add_configs("std_format",      {description = "Use std::format instead of fmt library.", default = false, type = "boolean"})
    add_configs("fmt_external",    {description = "Use external fmt library instead of bundled.", default = false, type = "boolean"})
    add_configs("fmt_external_ho", {description = "Use external fmt header-only library instead of bundled.", default = false, type = "boolean"})
    add_configs("noexcept",        {description = "Compile with -fno-exceptions. Call abort() on any spdlog exceptions.", default = false, type = "boolean"})

    on_load(function (package)
        if not package:config("header_only") then
            package:add("defines", "SPDLOG_COMPILED_LIB")
            package:add("deps", "cmake")
        end
        assert(not (package:config("fmt_external") and package:config("fmt_external_ho")), "fmt_external and fmt_external_ho are mutually exclusive")
        if package:config("std_format") then
            package:add("defines", "SPDLOG_USE_STD_FORMAT")
        elseif package:config("fmt_external") then
            package:add("defines", "SPDLOG_FMT_EXTERNAL")
            package:add("deps", "fmt")
        elseif package:config("fmt_external_ho") then
            package:add("defines", "SPDLOG_FMT_EXTERNAL_HO")
            package:add("deps", "fmt", {configs = {header_only = true}})
        end
        if package:config("noexcept") then
            package:add("defines", "SPDLOG_NO_EXCEPTIONS")
        end
        if package:config("wchar") then
            package:add("defines", "SPDLOG_WCHAR_TO_UTF8_SUPPORT")
        end
    end)

    on_install(function (package)
        if package:config("header_only") then
            os.cp("include", package:installdir())
            return
        end

        local configs = {"-DSPDLOG_BUILD_TESTS=OFF", "-DSPDLOG_BUILD_EXAMPLE=OFF"}
        table.insert(configs, "-DSPDLOG_BUILD_SHARED=" .. (package:config("shared") and "ON" or "OFF"))
        table.insert(configs, "-DSPDLOG_USE_STD_FORMAT=" .. (package:config("std_format") and "ON" or "OFF"))
        if(package:config("std_format")) then
            table.insert(configs, "-DCMAKE_CXX_STANDARD=20")
        end
        table.insert(configs, "-DSPDLOG_FMT_EXTERNAL=" .. (package:config("fmt_external") and "ON" or "OFF"))
        table.insert(configs, "-DSPDLOG_FMT_EXTERNAL_HO=" .. (package:config("fmt_external_ho") and "ON" or "OFF"))
        table.insert(configs, "-DSPDLOG_NO_EXCEPTIONS=" .. (package:config("noexcept") and "ON" or "OFF"))
        table.insert(configs, "-DSPDLOG_WCHAR_SUPPORT=" .. (package:config("wchar") and "ON" or "OFF"))
        import("package.tools.cmake").install(package, configs)
        os.trycp("include", package:installdir())
    end)

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
            table.insert(configs, "-Dprotobuf_MSVC_STATIC_RUNTIME=" .. (package:config("runtimes"):startswith("MT") and "ON" or "OFF"))
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