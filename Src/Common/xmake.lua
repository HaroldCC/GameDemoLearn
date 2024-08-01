add_requires("protobuf", "toml++")
add_requires("spdlog", {configs={std_format=true, header_only=false}})

target("Common")
    set_kind("static")
    add_headerfiles("**.h")
    add_files("**.cpp", "**.c", "Net/Proto/*.proto")

    add_rules("CommonRule", "protobuf.cpp")
    add_deps("asio", "magic_enum")

    add_packages("protobuf", "toml++")
    add_packages("spdlog", {public=true})

    add_includedirs("$(projectdir)/Src", "$(projectdir)/3rdParty/mysql/include", {public = true})

    add_linkdirs("$(projectdir)/3rdParty/mysql/lib")
    add_links("libmysql")

    after_build(function (target) 
        if is_plat("windows") then
            os.trycp("$(projectdir)/3rdParty/mysql/lib/*.dll", target:targetdir())
        end
    end)