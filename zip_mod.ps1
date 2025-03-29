
$dir = $PSScriptRoot
$downloads_path = "$HOME\downloads"
$zip_path = "$downloads_path\FreeLockOnCamera.zip"

Write-Host "Running build.ps1..."
& "$dir\build.ps1"
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build failed. Aborting."
    exit 1
}

Push-Location $dir
7z a $zip_path "mods\FreeLockOnCamera\config.ini" "mods\FreeLockOnCamera.dll" "mods\FreeLockOnCamera.pdb"
Pop-Location

Write-Host "Archive created at: $zip_path"

