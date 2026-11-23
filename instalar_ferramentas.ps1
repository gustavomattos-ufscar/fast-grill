# ---------------------------------------------------------------------------
# instalar_ferramentas.ps1
#
# Instala, SEM precisar de administrador, as duas ferramentas do projeto:
#   1) w64devkit 2.10.1 -> compilador gcc + make para Windows (portatil)
#   2) raylib 5.5       -> biblioteca grafica (versao para MinGW-w64, 64 bits)
#
# Tudo vai para C:\dev  (nada e instalado no sistema, so arquivos copiados).
# No final, C:\dev\w64devkit\bin e adicionado ao PATH do SEU usuario,
# para que "gcc" e "make" funcionem em qualquer terminal novo.
#
# Como rodar (PowerShell):
#   powershell -ExecutionPolicy Bypass -File instalar_ferramentas.ps1
# ---------------------------------------------------------------------------

$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'   # deixa o download muito mais rapido
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

$destino = 'C:\dev'
New-Item -ItemType Directory -Force -Path $destino | Out-Null

# 1) compilador (w64devkit): e um .exe auto-extraivel (formato 7z)
$devkitExe = Join-Path $destino 'w64devkit-x64-2.10.1.7z.exe'
if (-not (Test-Path (Join-Path $destino 'w64devkit\bin\gcc.exe'))) {
    Write-Host 'Baixando w64devkit (gcc + make, ~65 MB)...'
    Invoke-WebRequest -Uri 'https://github.com/skeeto/w64devkit/releases/download/v2.10.1/w64devkit-x64-2.10.1.7z.exe' -OutFile $devkitExe
    Write-Host 'Extraindo w64devkit em C:\dev\w64devkit ...'
    $p = Start-Process -FilePath $devkitExe -ArgumentList '-y', "-o$destino" -Wait -PassThru
    if ($p.ExitCode -ne 0) { throw "Falha ao extrair o w64devkit (codigo $($p.ExitCode))" }
    Remove-Item $devkitExe
} else {
    Write-Host 'w64devkit ja instalado.'
}

# 2) raylib 5.5 para MinGW-w64
$raylibDir = Join-Path $destino 'raylib-5.5_win64_mingw-w64'
if (-not (Test-Path (Join-Path $raylibDir 'lib\libraylib.a'))) {
    $raylibZip = Join-Path $destino 'raylib.zip'
    Write-Host 'Baixando raylib 5.5 (~2 MB)...'
    Invoke-WebRequest -Uri 'https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_win64_mingw-w64.zip' -OutFile $raylibZip
    Expand-Archive -Path $raylibZip -DestinationPath $destino -Force
    Remove-Item $raylibZip
} else {
    Write-Host 'raylib ja instalada.'
}

# 3) PATH do usuario (so do seu usuario, nao do sistema)
$bin = Join-Path $destino 'w64devkit\bin'
$pathUsuario = [Environment]::GetEnvironmentVariable('Path', 'User')
if ($pathUsuario -notlike "*$bin*") {
    [Environment]::SetEnvironmentVariable('Path', "$pathUsuario;$bin", 'User')
    Write-Host "Adicionado ao PATH do usuario: $bin"
}

Write-Host ''
Write-Host '=== Conferindo ==='
& (Join-Path $bin 'gcc.exe') --version | Select-Object -First 1
& (Join-Path $bin 'make.exe') --version | Select-Object -First 1
Write-Host "raylib: $(Test-Path (Join-Path $raylibDir 'include\raylib.h'))"
Write-Host 'Pronto! Feche e abra o terminal para o PATH novo valer.'
