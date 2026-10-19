param(
    [switch]$Shell,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
$root = $PSScriptRoot

if ($Shell) {
    docker run --rm -it -v "${root}:/work" -w /work ps1-builder bash
    exit
}

Write-Host "== Cartas do Destino - PS1 build ==" -ForegroundColor Cyan

$buildScript = @'
set -e
rm -rf build CARTAS.bin CARTAS.cue CARTAS.iso
cmake --preset default
cmake --build --preset default
cp -f build/GAME.exe iso_root/GAME.EXE
mkpsxiso -y iso.xml
ls -la CARTAS.bin CARTAS.cue
'@

$tmp = Join-Path $env:TEMP "ps1_build_$([guid]::NewGuid().ToString('N')).sh"
[System.IO.File]::WriteAllText($tmp, $buildScript, (New-Object System.Text.UTF8Encoding($false)))

try {
    docker run --rm -v "${root}:/work" -w /work -v "${tmp}:/build.sh:ro" ps1-builder bash /build.sh
    if ($LASTEXITCODE -ne 0) { throw "build failed" }
} finally {
    Remove-Item $tmp -ErrorAction SilentlyContinue
}

$bin = Join-Path $root "CARTAS.bin"
$cue = Join-Path $root "CARTAS.cue"
if (Test-Path $bin) {
    $size = (Get-Item $bin).Length
    Write-Host ""
    Write-Host "== SUCCESS ==" -ForegroundColor Green
    Write-Host "Imagem: $bin ($([math]::Round($size/1KB,1)) KB)"
    Write-Host "Cue:    $cue"
    Write-Host ""
    Write-Host "No DuckStation: File > Open > CARTAS.cue (ou CARTAS.bin)"
} else {
    throw "Image not generated"
}
