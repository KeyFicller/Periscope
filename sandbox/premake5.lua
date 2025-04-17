project "sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "on"

    targetdir(dir_workspace .. "/bin/" .. dir_output .. "/%{prj.name}")
    objdir(dir_workspace .. "/bin-int/" .. dir_output .. "/%{prj.name}")

    defines {

    }

    files {
        dir_include .. "/**.h",
        "inc/**.h",
        "src/**.cpp",
        "premake5.lua"
    }

    includedirs {
        dir_include,
        "inc",
    }

    links {
    }

