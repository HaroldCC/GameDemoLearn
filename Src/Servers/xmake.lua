target("HttpServer")
    set_kind("binary")
    add_rules("CommonRule")
    add_headerfiles("HttpServer/*.h")
    add_files("HttpServer/*.cpp")

    add_deps("Common")