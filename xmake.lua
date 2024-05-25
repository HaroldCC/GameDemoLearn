set_languages("c++20")

set_targetdir("Bin/$(plat)-$(arch)-$(mode)")

add_rules("mode.release", "mode.debug")
add_rules("plugin.vsxmake.autoupdate")
add_rules("plugin.compile_commands.autoupdate", {outputdir="$(projectdir)/.vscode", lsp="cland"})
set_exceptions("cxx")

if is_plat("windows") then
    before_build(function (target) 
        target:add("defines", "_WIN32_WINDOWS", "_WINSOCK_DEPRECATED_NO_WARNINGS")
    end)
end

rule("CommonRule")
    on_load(function (target) 
        if is_mode("debug") then
            target:add("defines", "DEBUG", "ENABLE_PERFORMANCE_DECT")
            target:set("symbols", "debug")
            target:set("optimize", "none")
        end

        target:set("warnings", "all")

        if target:is_plat("windows") then
            target:add("defines", "WIN32", "WIN32_LEAN_AND_MEAN", "ASIO_HAS_CO_AWAIT=1")
            target:add("cxxflags", "cl::/wd4819")
            target:add("defines", "_CRT_SECURE_NO_WARNINGS")
        end
    end)
rule_end()

includes("3rdParty")
includes("Src/Common")
includes("Src/Servers")

includes("Tests")