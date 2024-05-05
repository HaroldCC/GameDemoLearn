add_requires("protobuf-cpp")

target("Common")
    set_kind("static")
    add_headerfiles("**.h")
    add_files("**.cpp", "**.c", "Net/Proto/*.proto")

    add_rules("CommonRule", "protobuf.cpp")
    add_deps("asio", "spdlog", "magic_enum", "async_simple", {public=true})

    add_packages("protobuf-cpp")

    add_includedirs("$(projectdir)/Src", {public = true})