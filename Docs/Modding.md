# Asteria 모딩 가이드 (기본)

모드 = **콘텐츠 전용 플러그인**. 엔진이 `<프로젝트>/Mods/` 폴더의 플러그인을 Mod 타입으로 자동 인식·마운트한다.
게임 쪽 진입점은 `UAsteriaModSubsystem`(`Source/Asteria/Mod/`) 하나다.

## 모드 만들기 (에디터)

1. Edit → Plugins → **Add** → *Content Only* 로 플러그인 생성 (이름 = 모드 이름).
2. 생성된 폴더를 `Plugins/<ModName>/` 에서 `Mods/<ModName>/` 로 옮기고 에디터 재시작.
3. 모드 콘텐츠 루트(`/<ModName>/`)에 **`ModEntry`** 라는 이름의 Actor 블루프린트를 만든다.
   - 게임 월드가 시작되면 **서버(호스트)가** 모드마다 `ModEntry`를 하나 스폰한다. 모드 로직은 그 BeginPlay에서 시작.
   - 클라이언트에도 보여야 하면 `ModEntry`의 **Replicates**를 켠다. 상태 변경은 서버 권위 규칙을 그대로 따른다.
   - `ModEntry`가 없는 모드는 에셋 덮어쓰기 전용으로 취급된다(로그에 `ModEntry none`).
4. PIE로 확인: 출력 로그에서 `LogAsteriaMod` 필터.

## 배포 (패키징된 게임)

1. Project Launcher에서 기준 게임 릴리스 버전을 지정하고 모드 플러그인을 **DLC로 쿡/패키징**.
2. 결과물(`<ModName>.uplugin` + `Content/Paks/<Platform>/*.pak`)을 게임의 `Asteria/Mods/<ModName>/` 에 넣는다.

## 제약

- 코옵: **호스트와 모든 클라이언트가 같은 모드를 설치**해야 한다. 버전/목록 일치 검사는 아직 없다.
- 코드(C++) 모드는 지원하지 않는다 — 블루프린트·에셋만.
