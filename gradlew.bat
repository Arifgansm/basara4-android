@echo off
:: Windows Gradle Wrapper
if exist gradle.bat (
    call gradle.bat %*
) else (
    echo ERROR: Gradle not found.
)