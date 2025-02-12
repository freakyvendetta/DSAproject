@echo off
echo ====================================
echo  Building Pac-Man Arcade Simulation
echo ====================================
g++ -O2 -std=c++11 -o pacman.exe pacmansimulation.cpp
if %ERRORLEVEL% EQU 0 (
    echo [SUCCESS] Build completed! Run .\pacman.exe to play.
) else (
    echo [ERROR] Build failed! Check compiler output.
)
