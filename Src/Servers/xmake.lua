-- target("HttpServer")
--     set_kind("binary")
--     add_rules("CommonRule")
--     add_headerfiles("HttpServer/*.h")
--     add_files("HttpServer/*.cpp")

--     add_deps("Common")

target("LoginServer")
    set_kind("binary")
    add_rules("CommonRule")
    add_headerfiles("LoginServer/*.h")
    add_files("LoginServer/*.ixx", {public=true})
    add_files("LoginServer/*.cpp")
    add_deps("Common")