add_rules("mode.debug", "mode.release")

set_languages("c++26")
set_rundir("$(projectdir)")
set_policy("build.c++.modules", true)

target(
    "cpp-style-ref",
    function ()
        set_kind("binary")

        add_files("src/main.cpp")
        add_files("**.cppm")
    end
)

target(
    "error",
    function ()
        set_kind("static")

        add_files("src/error.cppm")
        add_files("src/error.cpp")
    end
)
