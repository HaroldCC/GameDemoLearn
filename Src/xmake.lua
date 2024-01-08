target("Common")
    set_kind("static")
    add_headerfiles("**.h")
    add_files("**.cpp")

    add_rules("CommonRule")
    add_deps("asio", "spdlog")