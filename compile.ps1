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
    "/arch:AVX512",
    "/diagnostics:caret",
    "/DNDEBUG",
    "/D_NDEBUG",
    "/EHa",
    "/F0x10485100",
    "/favor:INTEL64",
    "/fp:strict",
    "/fpcvt:IA",
    "/GL",
    "/Gw",
    "/I./",
    "/I./include/",
    "/jumptablerdata",
    "/MP",
    "/MT", # statically link the multithreaded version of Windows libs
    "/O2",
    "/Ob3",
    "/Oi",
    "/Ot",
    "/Qpar",
    "/std:c++20",
    "/TP",
    "/Wall",
    "/wd4505",      # removed unreferenced inline function
    "/wd4710",      # not inlined
    "/wd4711",      # selected for inline expansion
    "/wd4820",      # struct padding
    "/Zc:__cplusplus",
    "/Zc:preprocessor",
    "/link /DEBUG:NONE"
)

Write-Host "cl.exe ${cfiles} ${cflags}" -ForegroundColor Cyan
cl.exe $cfiles $cflags

Get-ChildItem *.obj -Recurse | Foreach-Object {Remove-Item $_.FullName -Force}
Get-ChildItem *.o   -Recurse | Foreach-Object {Remove-Item $_.FullName -Force}
Get-ChildItem *.pdb -Recurse | Foreach-Object {Remove-Item $_.FullName -Force}
Get-ChildItem *.exp -Recurse | Foreach-Object {Remove-Item $_.FullName -Force}
Get-ChildItem *.dll -Recurse | Foreach-Object {Remove-Item $_.FullName -Force}
Get-ChildItem *.lib -Recurse | Foreach-Object {Remove-Item $_.FullName -Force}
Get-ChildItem *.ilk -Recurse | Foreach-Object {Remove-Item $_.FullName -Force}
Get-ChildItem *.i   -Recurse | Foreach-Object {Remove-Item $_.FullName -Force}
