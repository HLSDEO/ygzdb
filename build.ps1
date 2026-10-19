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

$buildScript = @"
set -e
$(if ($Clean) { "rm -rf build CARTAS.iso" })
rm -rf build
cmake --preset default
cmake --build --preset default
cp -f build/GAME.exe iso_root/GAME.EXE
mkpsxiso -y iso.xml
ls -la CARTAS.iso
"@

$tmp = Join-Path $env:TEMP "ps1_build_$([guid]::NewGuid().ToString('N')).sh"
[System.IO.File]::WriteAllText($tmp, $buildScript, (New-Object System.Text.UTF8Encoding($false)))

try {
    docker run --rm -v "${root}:/work" -w /work -v "${tmp}:/build.sh:ro" ps1-builder bash /build.sh
    if ($LASTEXITCODE -ne 0) { throw "build failed" }
} finally {
    Remove-Item $tmp -ErrorAction SilentlyContinue
}

$iso = Join-Path $root "CARTAS.iso"
if (Test-Path $iso) {
    $size = (Get-Item $iso).Length
    Write-Host ""
    Write-Host "== SUCCESS ==" -ForegroundColor Green
    Write-Host "ISO: $iso ($([math]::Round($size/1KB,1)) KB)"
    Write-Host ""
    Write-Host "Teste no DuckStation / PCSX-Redux / Mednafen:"
    Write-Host "  File > Open > $iso"
} else {
    throw "ISO not generated"
}
