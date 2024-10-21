
$wd = Get-Location
Set-Location "c:\steam\steamapps\common\elden ring\modding\freelockoncamera"

$modName           = "FreeLockOnCamera"
$projectFile       = "./SourceCode/" + $modName + ".vcxproj"

$destinationFolder = "./mods/"
$sourceFolder      = "./SourceCode/x64/Release/"

if ($args.Length -gt 0) {
    msbuild $projectFile /p:Configuration=Release /p:Platform=x64 /t:Rebuild
} else {
    msbuild $projectFile /p:Configuration=Release /p:Platform=x64 $args
}
Copy-Item -Path ($sourceFolder + $modName + ".dll") -Destination $destinationFolder -Force
Copy-Item -Path ($sourceFolder + $modName + ".pdb") -Destination $destinationFolder -Force

Set-Location $wd

