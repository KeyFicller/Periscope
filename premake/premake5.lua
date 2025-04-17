project "rebuild_project_with_premake"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    targetdir(dir_workspace .. "/bin/" .. dir_output .. "/%{prj.name}")
    objdir(dir_workspace .. "/bin-int/" .. dir_output .. "/%{prj.name}")

    postbuildcommands {
        "cd ..",
        "call build.bat"
    }

    files {
        "premake5.lua"
    }