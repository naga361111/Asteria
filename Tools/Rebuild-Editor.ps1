<#
.SYNOPSIS
    핫 리로드 더미를 전부 지우고 에디터 모듈을 풀 빌드한다.

.DESCRIPTION
    stale reinstanced 클래스(REINST_/SKEL_/HOTRELOADED_)나 반영 안 된 UPROPERTY가
    의심될 때 쓴다. 디스크에 남는 원인은 두 군데뿐이다:
      1) Binaries/Win64 : UnrealEditor-Asteria*.dll/.pdb + UnrealEditor.modules
      2) Intermediate/Build : UHT 생성 헤더와 uhttimestamps (남아있으면 재생성을 스킵한다)
    둘 다 지운 뒤 UBT를 돌린다.

    건드리지 않는 것: 패키징 산출물(Asteria.exe/.pdb/.target), DerivedDataCache, Saved.
    커버 범위는 프로젝트 모듈(Asteria) 하나다. 나중에 Plugins/ 밑에 C++ 모듈을 만들면
    그쪽 Binaries/Intermediate도 같은 병을 앓으므로 삭제 목록에 추가해야 한다.

.EXAMPLE
    .\Tools\Rebuild-Editor.ps1 -WhatIf
    뭘 지울지만 확인하고 빌드는 하지 않는다.

.EXAMPLE
    .\Tools\Rebuild-Editor.ps1
#>
[CmdletBinding(SupportsShouldProcess)]
param(
    # 자동 탐색(uproject의 EngineAssociation → 레지스트리 → 기본 설치 경로)이 실패할 때만 지정.
    [string]$EnginePath,

    [ValidateSet('Development', 'DebugGame')]
    [string]$Configuration = 'Development'
)

$ErrorActionPreference = 'Stop'

$root    = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$project = Join-Path $root 'Asteria.uproject'
$target  = 'AsteriaEditor'

# --- 1. 락 확인 -------------------------------------------------------------
# 에디터가 떠 있으면 활성 DLL 삭제만 실패해서 "일부만 지워진 + .modules 불일치"라는
# 최악의 상태가 남는다. 지우기 전에 끊는다.
$locking = Get-Process -Name 'UnrealEditor', 'Asteria' -ErrorAction SilentlyContinue
if ($locking) {
    throw "DLL을 잡고 있는 프로세스가 있다: $(($locking.Name | Sort-Object -Unique) -join ', '). 먼저 종료할 것."
}

# --- 2. 엔진 위치 -----------------------------------------------------------
if (-not $EnginePath) {
    $version = (Get-Content $project -Raw | ConvertFrom-Json).EngineAssociation
    $key     = "HKLM:\SOFTWARE\EpicGames\Unreal Engine\$version"
    $EnginePath = if (Test-Path $key) {
        (Get-ItemProperty $key).InstalledDirectory
    } else {
        "C:\Program Files\Epic Games\UE_$version"
    }
}
$buildBat = Join-Path $EnginePath 'Engine\Build\BatchFiles\Build.bat'
if (-not (Test-Path $buildBat)) {
    throw "엔진을 찾을 수 없다: $buildBat`n-EnginePath 로 직접 지정할 것."
}

# --- 3. 삭제 ----------------------------------------------------------------
$doomed = @(
    Get-ChildItem (Join-Path $root 'Binaries\Win64') -File -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -like 'UnrealEditor-Asteria*' -or $_.Name -eq 'UnrealEditor.modules' }
    Get-Item (Join-Path $root 'Intermediate\Build') -ErrorAction SilentlyContinue
)

Write-Host "`n[1/2] 정리" -ForegroundColor Cyan
if ($doomed) {
    $doomed | ForEach-Object { Write-Host "      - $($_.FullName.Replace("$root\", ''))" }
    $doomed | Remove-Item -Recurse -Force
} else {
    Write-Host '      (이미 깨끗함)'
}

# --- 4. 풀 빌드 -------------------------------------------------------------
Write-Host "`n[2/2] 풀 빌드 — $target Win64 $Configuration" -ForegroundColor Cyan
if ($PSCmdlet.ShouldProcess($target, 'UnrealBuildTool 풀 빌드')) {
    $sw = [Diagnostics.Stopwatch]::StartNew()
    & $buildBat $target Win64 $Configuration "-Project=$project" -WaitMutex -FromMsBuild
    $exit = $LASTEXITCODE
    $sw.Stop()

    if ($exit -ne 0) { throw "빌드 실패 (exit $exit) — $('{0:mm\:ss}' -f $sw.Elapsed) 경과" }
    Write-Host ("`n완료 — {0:mm\:ss}" -f $sw.Elapsed) -ForegroundColor Green
}
