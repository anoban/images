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
    "/DDEBUG",
    "/D_DEBUG",
    "/D__TEST__",
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
    "/MTd", # statically link the multithreaded debug version of Windows libs
    "/O2",
    "/Ob3",
    "/Oi",
    "/Ot",
    "/Qpar",
    "/Qspectre",
    "/std:c++14",
    "/TP",
    "/Wall",
    "/wd4514",      # removed unreferenced inline function
    "/wd4710",      # not inlined
    "/wd4711",      # selected for inline expansion
    "/wd4820",      # padding
    "/Zc:__cplusplus",
    "/Zc:preprocessor",
    "/link /DEBUG:FULL"
)

Write-Host "cl.exe ${cfiles} ${cflags}" -ForegroundColor Cyan
cl.exe $cfiles $cflags

Get-ChildItem *.obj -Recurse | Foreach-Object {Remove-Item $_.FullName -Force}
Get-ChildItem *.pdb -Recurse | Foreach-Object {Remove-Item $_.FullName -Force}
Get-ChildItem *.exp -Recurse | Foreach-Object {Remove-Item $_.FullName -Force}
