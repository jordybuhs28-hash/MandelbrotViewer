# build.ps1 - Build & Run Mandelbrot Lab with Extra Credit Features
param(
    [switch]$Run,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

# Ensure MSYS2 MinGW64 bin is in PATH (adjust path if needed)
$mingwBin = "C:\msys64\mingw64\bin"
if (Test-Path $mingwBin -and ($env:PATH -split ';') -notcontains $mingwBin) {
    $env:PATH = "$mingwBin;$env:PATH"
}

if ($Clean) {
    Write-Host "Cleaning build artifacts..."
    if (Test-Path "mandelbrot.exe") { Remove-Item "mandelbrot.exe" -Force }
    if (Get-Command make -ErrorAction SilentlyContinue) {
        make clean
    }
}

# Prefer Makefile if present
if (Test-Path "Makefile" -or Test-Path "makefile") {
    Write-Host "Building via Makefile..."
    make
} else {
    Write-Host "Building directly with g++..."
    g++ -std=c++17 mandelbrot.cpp -o mandelbrot -lsfml-graphics -lsfml-window -lsfml-system
}

if ($Run) {
    Write-Host "Launching Mandelbrot viewer..."
    Start-Process -NoNewWindow -FilePath "$PWD\mandelbrot.exe"
}
