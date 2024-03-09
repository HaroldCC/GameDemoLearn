add_requires("protobuf", "toml++")

target("Common")
    set_kind("static")
    add_headerfiles("**.h")
    add_files("**.cpp", "**.c", "Net/Proto/*.proto")

    add_rules("CommonRule", "protobuf.cpp")
    add_deps("asio", "spdlog", "magic_enum")

    add_packages("protobuf", "toml++")

    add_includedirs("$(projectdir)/Src", "$(projectdir)/3rdParty/mysql/include", {public = true})

    add_linkdirs("$(projectdir)/3rdParty/mysql/lib")
    add_links("libmysql")

    after_build(function (target) 
        if is_plat("windows") then
            os.cp("$(projectdir)/3rdParty/mysql/lib/libmysql.dll", target:targetdir())
        end
    end)