# Third-party Library Versions

| 라이브러리 | 버전 | 업데이트 날짜 | 저장소 |
|---|---|---|---|
| asio (standalone) | 1.38.0 | 2026-05-17 | https://github.com/chriskohlhoff/asio |
| spdlog | 1.17.0 | 2026-05-17 | https://github.com/gabime/spdlog |
| nlohmann/json | 3.12.0 | 2026-05-17 | https://github.com/nlohmann/json |

## 업데이트 방침

- **patch/minor** (x.Y.z): 빌드 후 이상 없으면 적용
- **major** (X.y.z): 체인지로그 확인 후 기존 코드 영향 검토
- asio: 네트워크 핵심 — 업데이트 시 비동기 API 변경 여부 확인
- spdlog: 로그 API는 안정적, 업데이트 부담 낮음
- nlohmann/json: API 매우 안정적, minor 업데이트 자유롭게 적용 가능
