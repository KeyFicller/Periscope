dir_workspace = os.getcwd()

dir_output = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

dir_include = dir_workspace .. "/include"

workspace "Periscope"
	configurations {
		"Debug",
		"Release"
	}
	startproject "sandbox"
	
	filter "system:windows"
		platforms {
			"x64"
		}

	filter "configurations: Debug"
		defines {
			"DEBUG"
		}
		symbols "On"
		optimize "Off"
	filter "configurations: Release"
		defines {
			"NODEBUG"
		}
		symbols "On"
		optimize "Speed"

include "sandbox"
include "premake"


