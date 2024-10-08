add_rules("mode.debug", "mode.release")  -- 添加 debug 和 release 模式
set_languages("cxx17")  -- 设置 C++ 标准
if is_mode("debug") then
    set_warnings("all")
    set_optimize("none")
    set_symbols("debug")   -- 打开调试符号
else
    set_optimize("fastest")  -- 设置快速优化
    set_symbols("hidden")
    set_strip("all")
    set_optimize("fastest")
end
target("binary")
    add_includedirs(
    "include/Log",
    "include/power",
    "include/config",
    "include/activity",
    "include/file"
    )
    set_arch("arm64-v8a")
    set_kind("binary")
    add_files("src/*.cpp")
target_end()
target("module")
    add_includedirs(
    "include/Log",
    "include/power",
    "include/config",
    "include/activity",
    "include/file"
    )
    set_arch("arm64-v8a")
    set_kind("binary")
    add_files("src/*.cpp")
    after_build(function (target)
        import("core.project.project")
        -- 获取编译产物的路径
        local output_dir = target:targetdir()
        local output_file = target:targetfile()
        os.cp("MagiskModule",output_dir)
        os.cp(output_file,output_dir.."/MagiskModule")
        os.cd(output_dir)
        os.rm("module.zip")
        os.rm("module")
        os.cd("MagiskModule")
        os.exec("zip -r ../module.zip .")
        os.cd("..")
        os.rm("MagiskModule/")
    end)
target_end()

-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro definition
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--

