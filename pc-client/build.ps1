param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",
    [string]$QtInstallRoot = "D:\Qt",
    [switch]$Test,
    [switch]$Package
)

$qtSdkDirectory = Join-Path $QtInstallRoot "6.8.3\mingw_64"
$mingwDirectory = Join-Path $QtInstallRoot "Tools\mingw1310_64"
$cmakeExecutable = Join-Path $QtInstallRoot "Tools\CMake_64\bin\cmake.exe"
$ninjaExecutable = Join-Path $QtInstallRoot "Tools\Ninja\ninja.exe"
$compilerExecutable = Join-Path $mingwDirectory "bin\g++.exe"
$buildDirectory = Join-Path $PSScriptRoot ("build-" + $Configuration.ToLowerInvariant())
$packageDirectory = Join-Path $PSScriptRoot ("dist\" + $Configuration)

$requiredPaths = @(
    $qtSdkDirectory,
    $cmakeExecutable,
    $ninjaExecutable,
    $compilerExecutable
)

foreach ($requiredPath in $requiredPaths) {
    if (-not (Test-Path -LiteralPath $requiredPath)) {
        throw "缺少构建环境：$requiredPath"
    }
}

$savedProcessPath = $env:Path
try {
    $env:Path = (Join-Path $mingwDirectory "bin") + ";" `
        + (Join-Path $qtSdkDirectory "bin") + ";" `
        + (Split-Path -Parent $ninjaExecutable) + ";" `
        + $savedProcessPath

    & $cmakeExecutable `
        -S $PSScriptRoot `
        -B $buildDirectory `
        -G Ninja `
        "-DCMAKE_MAKE_PROGRAM=$ninjaExecutable" `
        "-DCMAKE_CXX_COMPILER=$compilerExecutable" `
        "-DCMAKE_PREFIX_PATH=$qtSdkDirectory" `
        "-DCMAKE_BUILD_TYPE=$Configuration"
    if ($LASTEXITCODE -ne 0) {
        throw "CMake 配置失败，退出码：$LASTEXITCODE"
    }

    & $cmakeExecutable --build $buildDirectory --parallel
    if ($LASTEXITCODE -ne 0) {
        throw "编译失败，退出码：$LASTEXITCODE"
    }

    if ($Test) {
        $ctestExecutable = Join-Path (Split-Path -Parent $cmakeExecutable) "ctest.exe"
        & $ctestExecutable --test-dir $buildDirectory --output-on-failure
        if ($LASTEXITCODE -ne 0) {
            throw "测试失败，退出码：$LASTEXITCODE"
        }
    }

    if ($Package) {
        & $cmakeExecutable --install $buildDirectory --prefix $packageDirectory
        if ($LASTEXITCODE -ne 0) {
            throw "打包失败，退出码：$LASTEXITCODE"
        }
        Write-Output "可分发程序：$packageDirectory\bin\RvmSerialClient.exe"
    } else {
        Write-Output "构建完成：$buildDirectory\RvmSerialClient.exe"
    }
} finally {
    $env:Path = $savedProcessPath
}
