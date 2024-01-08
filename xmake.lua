set_languages("c++20")

set_targetdir("Bin/$(plat)-$(arch)-$(mode)")

add_rules("mode.release", "mode.debug")
add_rules("plugin.vsxmake.autoupdate")
add_rules("plugin.compile_commands.autoupdate", {outputdir="$(projectdir)/.vscode", lsp="cland"})

-- add_requires("protobuf-cpp", "swig")

rule("CommonRule")
    on_load(function (target) 
        if is_mode("debug") then
            target:add("defines", "DEBUG", "PERFORMANCE_DECT")
            target:set("suffixname", "_d")
            target:set("symbols", "debug")
            target:set("optimize", "none")
        end

        target:set("warnings", "all", "error")

        if target:is_plat("windows") then
            target:add("defines", "WIN32", "WIN32_LEAN_AND_MEAN")
            target:add("cxxflags", "cl::/wd4819")
            target:add("defines", "_CRT_SECURE_NO_WARNINGS")
        end
    end)
rule_end()

includes("Src")
includes("Tests")
includes("3rdParty")