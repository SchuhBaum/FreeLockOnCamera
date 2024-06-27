$modName           = "FreeLockOnCamera"
$projectFile       = "./SourceCode/" + $modName + ".vcxproj"

$destinationFolder = "./mods/"
$sourceFolder      = "./SourceCode/x64/Release/"

msbuild $projectFile /p:Configuration=Release /p:Platform=x64
Copy-Item -Path ($sourceFolder + $modName + ".dll") -Destination $destinationFolder -Force
Copy-Item -Path ($sourceFolder + $modName + ".pdb") -Destination $destinationFolder -Force

