$ErrorActionPreference='Stop'
# Run from the repository root; build products stay ignored.
$vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
$installation=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if(!$installation){throw 'Visual C++ x86 tools not found'}
$vcvars=Join-Path $installation 'VC/Auxiliary/Build/vcvars32.bat'
New-Item -ItemType Directory -Force build | Out-Null
$commands=@"
@call "$vcvars" >nul
@if errorlevel 1 exit /b 1
@cl /nologo /std:c++latest /EHsc /MT /O2 tools\controller-binding-defaults.cpp /Fe:build\controller-binding-defaults.exe /Fo:build\controller-binding-defaults.obj
@if errorlevel 1 exit /b 1
@build\controller-binding-defaults.exe
@if errorlevel 1 exit /b 1
@cl /nologo /std:c++latest /EHsc /MT /O2 tools\tests\vrbindingconfig.cpp /Fe:build\vrbindingconfig-test.exe /Fo:build\vrbindingconfig-test.obj
@if errorlevel 1 exit /b 1
@build\vrbindingconfig-test.exe
@exit /b %errorlevel%
"@
Set-Content -LiteralPath build/test-controller-bindings.cmd -Value $commands -Encoding ascii
& .\build\test-controller-bindings.cmd
if($LASTEXITCODE -ne 0){throw 'Controller binding checks failed'}
