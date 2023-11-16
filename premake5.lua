
local function addSrcFiles( prefix )
	return prefix .. "/*cpp", prefix .. "/*.h"
end


workspace "MHWSF"
	configurations { "Release" }
	location "build"

	files { addSrcFiles("src") }
	
	includedirs { "src" }
	includedirs { "inc" }

project "MHWSF"
	kind "SharedLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	targetextension ".asi"
	characterset ("MBCS")

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		flags { "StaticRuntime" }
