-- Premake5 scriptfile created by Nicolaus Anderson, Nov 10, 2018
-- Usage: premake5 --file=premake5.lua [gmake, vs2015, ..]
-- The make config=[some configuration] requires that the configuration be in lowercase letters.

local v_cubr_path = "../CupricBridge/src/"
local v_copper_path = "../CopperLang/Copper/src/"
local v_copper_stdlib_path = "../CopperLang/Copper/stdlib/"
--local v_irrext_path = "../../IrrExtensions/"
local v_irrlicht_home = "/usr/local"
local v_irrlicht_include = "/usr/local/include/irrlicht/"
local v_freetype_include = "/usr/include/freetype2"

-- "make" paths
local v_b_cubr_path = "../" .. v_cubr_path
local v_b_copper_path = "../" .. v_copper_path
local v_b_copper_stdlib_path = "../" .. v_copper_stdlib_path
--local v_b_irrext_path = "../" .. v_irrext_path


workspace "Curri App"
	configurations	{ "debug", "release" }
	location		"build"
	objdir			"build/obj"
	--targetdir		"bin"
	targetdir		"."
	filter { "configurations: debug" }
		optimize "Off"
		warnings "Extra"
	filter { "configurations: release" }
		optimize "On"
	filter { "action:gmake" }
		buildoptions " -g -Wfatal-errors"

project "Curri App"
	targetname	"app.out"
	language	"C++"
	flags		{ "C++11" }
	kind		"ConsoleApp"
	links {
		"Irrlicht",
		"GL",
		"Xxf86vm",
		"Xext",
		"X11",
		"Xcursor",
		"freetype" -- For CGUITTFont
	}
	defines( "SYSTEM=Linux" )
	-- TODO: Should move buildoptions to includedirs if supported.
	includedirs { "src" }
	files {
		"src/**.h"
		, "src/**.cpp"
		, v_cubr_path .. "**.h"
		, v_cubr_path .. "**.cpp"
		--, v_irrext_path .. "**.h"
		--, v_irrext_path .. "**.cpp"
		, v_copper_path .. "**.h"
		, v_copper_path .. "**.cpp"
		, v_copper_stdlib_path .. "**.h"
		, v_copper_stdlib_path .. "**.cpp"
	}
	removefiles {
		"src/excludes/**.h"
		, "src/excludes/**.cpp"
		, v_cubr_path .. "excludes/**.h"
		, v_cubr_path .. "excludes/**.cpp"
	}
	buildoptions {
		"-I" .. v_b_cubr_path
		, "-I" .. v_b_copper_path
		, "-I" .. v_b_copper_stdlib_path
		, "-I" .. v_irrlicht_include
		--, "-I" .. v_b_irrext_path
		, "-I" .. v_freetype_include
	}
	linkoptions {
		" -L" .. v_irrlicht_home .. "/lib"
	}
