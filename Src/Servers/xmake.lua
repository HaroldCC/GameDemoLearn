target("HttpServer")
    set_kind("binary")
    add_rules("CommonRule")
    add_headerfiles("HttpServer/*.h")
    add_files("HttpServer/*.cpp")

    add_deps("Common")

target("DemoServer")
    set_kind("binary")
    add_rules("CommonRule")
    add_headerfiles("DemoServer/*.h")
    add_files("DemoServer/*.cpp")
    add_deps("Common")