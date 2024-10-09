add_requires("v8")

rule("swig.cpp2js")do
    set_extensions(".i")

    on_config(function (target) 
        local swigRule = target:sourcebatches()["swig.cpp2js"]

        for _, sourcefile in pairs(swigRule.sourcefiles or {}) do
            local baseSourceFile = path.basename(sourcefile)
            local outsourceFile = baseSourceFile..".cxx"
            local outputDir = path.join(target:scriptdir(), "Out")
            local outFullFile = path.join(outputDir, outsourceFile)
            if not os.isdir(outputDir) then
                os.mkdir(outputDir)
            end
        
            local args =
            {
                "-c++", 
                "-javascript",
                "-v8",
                "-I"..target:scriptdir(),
                "-o",
                outFullFile,
                path.join(os.projectdir(), sourcefile)
            }
            os.vrunv("$(projectdir)/Tools/swig/swig", args)

            target:add("files", outFullFile)
        end
    end)
end

target("TestSwigJS") do
    set_kind("shared")

    add_rules("swig.cpp2js")
    add_packages("v8")

    add_headerfiles("*.h")
    add_files("*.cpp", "*.i")
end