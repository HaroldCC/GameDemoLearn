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

    before_build(function (target) 
        if target:is_plat("windows") then
            target:add("defines", "_WIN32_WINDOWS", "_WINSOCK_DEPRECATED_NO_WARNINGS")
        end
    end)
target_end()

target("spdlog")
    set_kind("static")
    add_rules("3rdPartyRule")
    set_group("3rdParty")
    add_headerfiles("spdlog/include/spdlog/**.h", {prefixdir="spdlog"})
    add_includedirs("spdlog/include", {public = true})
    -- add_defines("SPDLOG_USE_STD_FORMAT")
    -- add_files("spdlog/src/**.cpp")

    on_load(function (target) 
        local commonSrc = 
        {
            "spdlog/src/async_logger.cpp",
            "spdlog/src/common.cpp",
            "spdlog/src/logger.cpp",
            "spdlog/src/pattern_formatter.cpp",
            "spdlog/src/spdlog.cpp",
            "spdlog/src/cfg/helpers.cpp",
            "spdlog/src/details/file_helper.cpp",
            "spdlog/src/details/log_msg.cpp",
            "spdlog/src/details/log_msg_buffer.cpp",
            "spdlog/src/details/periodic_worker.cpp",
            "spdlog/src/details/registry.cpp",
            "spdlog/src/details/thread_pool.cpp"    ,
            "spdlog/src/sinks/ansicolor_sink.cpp",
            "spdlog/src/sinks/base_sink.cpp",
            "spdlog/src/sinks/basic_file_sink.cpp",
            "spdlog/src/sinks/rotating_file_sink.cpp",
            "spdlog/src/sinks/sink.cpp",
            "spdlog/src/sinks/stdout_color_sinks.cpp",
            "spdlog/src/sinks/stdout_sinks.cpp",
        }

        for _, filename in pairs(commonSrc) do
            target:add("files", path.join("$(projectdir)/3rdParty", filename))
        end

        if(target:is_plat("windows")) then
            target:add("files", path.join("$(projectdir)/3rdParty", "spdlog/src/sinks/wincolor_sink.cpp"),
                                path.join("$(projectdir)/3rdParty", "spdlog/src/details/os_windows.cpp"))
            local winHeaders = 
            {
                "spdlog/include/spdlog/details/tcp_client_windows.h",
                "spdlog/include/spdlog/details/udp_client_windows.h",
                "spdlog/include/spdlog/details/windows_include.h",
                "spdlog/include/spdlog/sinks/win_eventlog_sink.h",
                "spdlog/include/spdlog/sinks/wincolor_sink.h",
            }
            for _, filename in pairs(winHeaders) do
                target:add("headerfiles", path.join("$(projectdir)/3rdParty", filename))
            end            
        else
            target:add("files", path.join("$(projectdir)/3rdParty", "spdlog/src/details/os_unix.cpp"))
            target:add("headerfiles", path.join("$(projectdir)/3rdParty", "spdlog/include/spdlog/details/tcp_client_unix.h"),
                                        path.join("$(projectdir)/3rdParty", "spdlog/include/spdlog/details/udp_client_unix.h"))
        end
    end)
target_end()

target("magic_enum")
    set_kind("headeronly")
    add_rules("3rdPartyRule")
    add_headerfiles("magic_enum/include/magic_enum/*.hpp", {prefixdir="magic_enum"})
    add_includedirs("magic_enum/include", {public = true})
target_end()

target("async_simple") do 
    set_kind("headeronly")
    add_rules("3rdPartyRule")
    add_headerfiles("async_simple/async_simple/**.h", {prefixdir="async_simple"})
    add_includedirs("async_simple", {public=true})
end