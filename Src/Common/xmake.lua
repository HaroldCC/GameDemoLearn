add_requires("protobuf")

target("Common")
    set_kind("static")
    add_headerfiles("**.h")
    add_files("**.cpp", "**.c", "Net/Proto/*.proto")

    add_rules("CommonRule", "protobuf.cpp")
    add_deps("asio", "spdlog", "magic_enum")

    add_packages("protobuf")

    add_includedirs("$(projectdir)/Src", {public = true})