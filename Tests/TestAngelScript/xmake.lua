target("TestAngelScript") do
    set_kind("binary")
    add_rules("TestRule")

    -- set_rundir("$(projectdir)/Bin/$(plat)-$(arch)-$(mode)/Tests")
    set_runargs("main.as")

    add_headerfiles("*.h")
    add_files("*.cpp")

    add_deps("angelscript")
end