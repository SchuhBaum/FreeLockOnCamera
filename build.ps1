
$dir = $PSScriptRoot
$ErrorActionPreference = "Stop"

$mod_name     = "FreeLockOnCamera"
$project_file = "$dir\sourcecode\" + $mod_name + ".vcxproj"

#
#

$src = "$dir\..\..\game\mods"
$dst = "$dir\mods"

New-Item -ItemType Directory -Path $src -Force | Out-Null
if ((Test-Path $src) -and -not (Test-Path $dst)) {
    New-Item -ItemType SymbolicLink -Path $dst -Target $src
}

#
#

$src = "$dir\sourcecode\x64\release"
$dst = "$dir\mods"

if ($args.Length -gt 0) {
    msbuild $project_file /p:Configuration=Release /p:Platform=x64 /t:Rebuild
} else {
    msbuild $project_file /p:Configuration=Release /p:Platform=x64 $args
}

Copy-Item -Path "$src\$mod_name.dll" -Destination $dst -Force
Copy-Item -Path "$src\$mod_name.pdb" -Destination $dst -Force

New-Item -ItemType Directory -Path "$dst\FreeLockOnCamera" -Force | Out-Null
Copy-Item -Path "$dir\config\config.ini" -Destination "$dst\FreeLockOnCamera" -Force

