<#
.SYNOPSIS
    プロジェクト内の全 HLSL シェーダーを DXC で構文検証する。

.DESCRIPTION
    ビルドに組み込まれた Windows SDK 同梱の dxc とは独立に、
    vcpkg でバージョン固定した dxc で検証を行う。
    バイナリは出力せず (-Fo nul)、診断のみを収集する。
    エラーまたは警告が 1 件でもあれば終了コード 1 を返す。

.PARAMETER ShaderModel
    検証に使うシェーダーモデル。既定は 6_0（ビルド設定と一致）。
    SM を上げる前の影響調査には -ShaderModel 6_6 を指定する。

.PARAMETER HLSLVersion
    HLSL 言語バージョン。既定は dxc の既定（2021）に従う。

.PARAMETER WarningsAsErrors
    警告をエラーとして扱う（-WX）。既定で有効。

.EXAMPLE
    .\tools\validate-shaders.ps1
    .\tools\validate-shaders.ps1 -ShaderModel 6_6
#>
[CmdletBinding()]
param(
    [string]$ShaderModel = "6_0",
    [string]$HLSLVersion = "",
    [bool]$WarningsAsErrors = $true
)

$ErrorActionPreference = 'Stop'

# --- パス解決（スクリプト位置基準）---------------------------------
$repoRoot   = Split-Path $PSScriptRoot -Parent
$dxc        = Join-Path $repoRoot "vcpkg_installed\x64-windows\tools\directx-dxc\dxc.exe"
$shaderRoot = Join-Path $repoRoot "assets\shader"

if (-not (Test-Path $dxc)) {
    Write-Error "dxc.exe が見つかりません: $dxc`nvcpkg install を実行してください。"
    exit 2
}
if (-not (Test-Path $shaderRoot)) {
    Write-Error "シェーダーディレクトリが見つかりません: $shaderRoot"
    exit 2
}

# --- コンパイル対象の定義（単一の情報源）---------------------------
# ファイルを追加したらここに 1 行足す。
# 1 ファイルに複数エントリポイントがある場合は複数行書く。
$jobs = @(
    @{ File = "TestVS.hlsl";               Stage = "vs"; Entry = "main" }
    @{ File = "UI_VS.hlsl";                Stage = "vs"; Entry = "main" }
    @{ File = "SkyboxVS.hlsl";             Stage = "vs"; Entry = "main" }
    @{ File = "GGX_PS.hlsl";               Stage = "ps"; Entry = "main" }
    @{ File = "UI_PS.hlsl";                Stage = "ps"; Entry = "main" }
    @{ File = "SkyboxPS.hlsl";             Stage = "ps"; Entry = "main" }
    @{ File = "EquirectToCubemapCS.hlsl";     Stage = "cs"; Entry = "main" }
)

# --- 定義漏れの検出 -------------------------------------------------
# ディレクトリ内の .hlsl と $jobs を突き合わせ、定義されていないものを警告。
# これがないと「追加したのに検証されていない」に気づけない。
$declared = $jobs | ForEach-Object { $_.File } | Sort-Object -Unique
$actual   = Get-ChildItem -Path $shaderRoot -Filter *.hlsl |
            ForEach-Object { $_.Name } | Sort-Object

$undeclared = $actual | Where-Object { $_ -notin $declared }
$missing    = $declared | Where-Object { $_ -notin $actual }

if ($undeclared) {
    Write-Warning "検証対象に未定義のシェーダーがあります: $($undeclared -join ', ')"
    Write-Warning "  → tools/validate-shaders.ps1 の `$jobs に追加してください。"
}
if ($missing) {
    Write-Warning "定義されているがファイルが存在しません: $($missing -join ', ')"
}

# --- 検証実行 -------------------------------------------------------
Write-Host "DXC : $((& $dxc --version) -join ' / ')"
Write-Host "SM  : $ShaderModel"
Write-Host ""

$errorCount   = 0
$warningCount = 0

foreach ($job in $jobs) {
    $path = Join-Path $shaderRoot $job.File
    if (-not (Test-Path $path)) { continue }

    $dxcArgs = @(
        "-T", "$($job.Stage)_$ShaderModel"
        "-E", $job.Entry
        "-I", $shaderRoot
        "-Fo", "nul"
    )
    if ($HLSLVersion)      { $dxcArgs += @("-HV", $HLSLVersion) }
    if ($WarningsAsErrors) { $dxcArgs += "-WX" }
    $dxcArgs += $path

    $output = & $dxc @dxcArgs 2>&1
    $ok     = ($LASTEXITCODE -eq 0)

    $errs  = @($output | Select-String -Pattern 'error:'   -SimpleMatch)
    $warns = @($output | Select-String -Pattern 'warning:' -SimpleMatch)
    $errorCount   += $errs.Count
    $warningCount += $warns.Count

    if ($ok -and $warns.Count -eq 0) {
        Write-Host ("  [OK]   {0} ({1})" -f $job.File, "$($job.Stage)_$ShaderModel") -ForegroundColor Green
    } else {
        Write-Host ("  [FAIL] {0} ({1})" -f $job.File, "$($job.Stage)_$ShaderModel") -ForegroundColor Red
        $output | ForEach-Object { Write-Host "         $_" -ForegroundColor DarkGray }
    }
}

Write-Host ""
Write-Host "$($jobs.Count) shaders : $errorCount errors, $warningCount warnings"

if ($errorCount -gt 0 -or $warningCount -gt 0 -or $undeclared) { exit 1 }
exit 0