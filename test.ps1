$cfiles = [System.Collections.ArrayList]::new()
$unrecognized = [System.Collections.ArrayList]::new()


foreach ($arg in $args) {
    if ($arg -clike "*.cpp") {
        [void]$cfiles.Add($arg.ToString().Replace(".\", ""))
    }
    else {
        [void]$unrecognized.Add($arg)
    }
}

if ($unrecognized.Count -ne 0) {
    Write-Error "Incompatible files passed for compilation: ${unrecognized}"
    Exit 1
}

$cflags = @(
    "./tests/main.cpp", # contains the main() function
    "./googletest/src/gtest-all.cc", # convenience source file that bundles all the necessary gtest sources
    "/arch:AVX512",
    "/diagnostics:caret",
    "/DNDEBUG",
    "/D_NDEBUG",
    "/EHa",
    "/F0x10485100",
    "/favor:INTEL64",
    "/Fe:test.exe",
    "/fp:strict",
    "/fpcvt:IA",
    "/GL",
    "/Gw",
    "/I./include/",
    "/I./googletest/",
    "/I./googletest/include/",
    "/jumptablerdata",
    "/MP",
    "/MT", # statically link the multithreaded version of Windows libs
    "/O2",
    "/Ob3",
    "/Oi",
    "/Ot",
    "/Qpar",
    "/std:c++14",
    "/TP",
    "/Wall",
    "/wd4514",      # removed unreferenced inline function
    "/wd4625",      # gtest
    "/wd4626",      # gtest
    "/wd4710",      # not inlined
    "/wd4711",      # selected for inline expansion
    "/wd4820",      # padding
    "/wd5026",      # gtest
    "/wd5027",      # gtest
    "/Zc:__cplusplus",
    "/Zc:preprocessor",
    "/link /DEBUG:NONE"
)

Write-Host "cl.exe ${cfiles} ${cflags}" -ForegroundColor Cyan
cl.exe $cfiles $cflags
