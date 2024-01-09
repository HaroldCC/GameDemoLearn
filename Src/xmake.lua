add_requires("protobuf")

target("Common")
    set_kind("static")
    add_headerfiles("**.h")
    add_files("**.cpp", "Common/Net/Proto/*.proto")

    add_rules("CommonRule", "protobuf.cpp")
    add_deps("asio", "spdlog")

    add_packages("protobuf")

    add_includedirs(".", {public = true})