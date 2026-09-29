---
name: pcg-filter-by-index-empty-crash
description: "UE 5.8 PCG \"Filter Elements By Index\"에 빈 데이터나 음수 인덱스가 들어가면 에디터가 크래시함 — 입력을 Merge Points로 합치고 분기(Branch)로 막을 것"
metadata:
  node_type: memory
  type: project
  originSessionId: 7922e10b-a6e2-4d6d-9487-1aef4f271de6
  modified: 2026-09-29T05:46:17.939Z
---

2026-09-29 PCG_GuildShell 문 4방향 작업 중 에디터가 두 번 크래시했다. 콜스택은 `PCGIndexing::FPCGIndexCollection::AddRange`이고, 오류는 "Trying to resize TArray to an invalid size of 4294967294"였다.
- 원인 1: 선택 인덱스 문자열이 "0"이어도 입력 데이터가 비어 있으면 크래시한다. 앞 노드가 빈 데이터 여러 개를 넘길 때 생긴다.
- 원인 2: 인덱스 속성 값이 -1이고 데이터가 비어 있지 않으면 크래시한다.

**Why:** 크래시 전에 저장하지 않은 그래프 편집은 전부 날아간다(서브그래프를 한 번 통째로 다시 만들었다).
**How to apply:** 여러 데이터가 들어오는 곳에는 먼저 Merge Points를 둔다. "고르지 않음"은 음수 인덱스가 아니라 Branch로 처리한다. MCP로 그래프를 고친 뒤 실행하기 전에 반드시 save_assets부터 한다. 관련: [[hearthvale-module-sizes]]
