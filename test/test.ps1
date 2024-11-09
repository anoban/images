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
    "./main.cpp",
    "./googletest/src/gtest-all.cc",
    "/arch:AVX512",
    "/diagnostics:caret",
    "/DDEBUG",
    "/D_DEBUG",
    "/D__TEST__",
    "/EHa",
    "/F0x10485100",
    "/favor:INTEL64",
    "/fp:strict",
    "/fpcvt:IA",
    "/GL",
    "/Gw",
    "/I./googletest",
    "/I./googletest/include",
    "/I./../include",
    "/jumptablerdata",
    "/MP",
    "/MTd",
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
    "/wd4820",      # struct padding
    "/wd4061",      # gtest
    "/wd4623",      # gtest
    "/wd4625",      # gtest
    "/wd4626",      # gtest
    "/wd4668",      # gtest
    "/wd5026",      # gtest
    "/wd5027",      # gtest
    "/wd5045",      # gtest
    "/Zc:__cplusplus",
    "/Zc:preprocessor",
    "/link /DEBUG:FULL"
)

Write-Host "cl.exe ${cfiles} ${cflags}" -ForegroundColor Cyan
cl.exe $cfiles $cflags

# If cl.exe returned 0, (i.e if the compilation succeeded,)

if ($? -eq $True){
    foreach($file in $cfiles){
        Remove-Item $file.Replace(".cpp", ".obj") -Force
    }
}
