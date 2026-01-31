# GitFlow Dashboard project architecture
_Exported on 06.01.2026 at 19:40:03 GMT+1 from Cursor (2.2.20)_

---

**User**

сделай архитекруту проекта но не начинай его писать 
Project Idea: GitFlow Dashboard - Multi-Interface GitHub Activity Monitor
Project Overview

A comprehensive GitHub activity monitoring dashboard with dual interfaces:

    TUI (Terminal UI) using C with Ncurses for terminal-based access

    Web Dashboard using TypeScript/React for modern web interface
Core Features

    Real-time monitoring of PRs, Issues, Code Reviews

    Personal and team activity statistics

    Goal tracking with gamification (streaks, achievements)

    Time analytics and productivity patterns

    Dual interface access (terminal + web)

Technology Stack

Backend/Core:

    C (system programming): Daemon, TUI, HTTP server

    Shared Memory/Unix Sockets for IPC

    Ncurses for terminal interface

    libcurl for GitHub API integration

Frontend:

    TypeScript/React for web interface

    WebSocket/HTTP for real-time updates

    Chart.js/D3.js for visualizations

Learning Objectives

    Master system programming in C (memory management, threads, sockets)

    Implement inter-process communication (shared memory, Unix sockets)

    Build terminal applications with Ncurses

    Create a full-stack application from low-level C to modern web

    Integrate with external APIs (GitHub REST/GraphQL)
Key Technical Challenges

    Efficient data synchronization between C daemon and multiple clients

    Real-time updates via WebSocket while maintaining performance

    Cross-language data serialization (C structs ↔ JSON ↔ TypeScript interfaces)

    Memory management in long-running C daemon

    Responsive TUI with ncurses and asynchronous data updates

Development Phases

Phase 1 (MVP - 4 weeks):

    Basic C daemon fetching GitHub data

    Simple TUI displaying key metrics

    File-based data persistence

Phase 2 (Full Integration - 8 weeks):

    HTTP API server in C

    React frontend with real-time updates

    Advanced caching strategy

Phase 3 (Advanced Features - 12 weeks):

    Team analytics and collaboration features

    Notifications and alerts

    Performance optimizations

    Deployment automation

Unique Aspects

    Dual-interface architecture - Same core data, two presentation layers

    Cross-paradigm development - Imperative C + declarative React

    Real-world API integration with rate limiting and error handling

    System programming meets web development in one project

    Practical utility - Solves actual developer productivity tracking needs

Expected Outcomes

    Production-ready monitoring tool

    Deep understanding of system programming concepts

    Portfolio piece demonstrating full-stack capabilities

    Open-source project with potential for community contribution

    Transferable skills to DevOps/SRE roles

Technical Specifications

    Languages: C (C11), TypeScript (ES2022)

    Libraries: Ncurses, libcurl, cJSON, React, Chart.js

    Protocols: HTTP/1.1, WebSocket, REST, GraphQL

    IPC: POSIX Shared Memory, Unix Domain Sockets

    Build Tools: Make, GCC, Node.js, Vite

    Deployment: Docker, systemd services

Success Metrics

    TUI updates within 100ms of data change

    Web interface loads in <2s

    Daemon memory usage <100MB

    Support for 10+ concurrent WebSocket connections

    GitHub API rate limit optimization

---

**Cursor**

```plaintext
**Defining project components**

The rules specify that for new code, I should use a language tag, though 'text' isn't officially listed. To be safe, I’ll use
```

### Архитектура GitFlow Dashboard (не код, только проектирование)

- **Цель**: единое ядро данных, два интерфейса (TUI и Web), минимальная задержка обновлений, безопасная и эффективная интеграция с GitHub.

### Высокоуровневые компоненты

- **`daemon (C)`**: долгоживущий процесс, тянет данные из GitHub, нормализует, кэширует, отдает через HTTP/WebSocket, обслуживает локальный IPC.
- **`tui (C + ncurses)`**: терминальный клиент, подключается к `daemon` через Unix-сокет, отрисовывает метрики и ленту активности.
- **`web (TypeScript/React)`**: SPA, получает данные по HTTP и реалтайм по WebSocket, визуализация (Chart.js/D3).
- **IPC**: Unix Domain Sockets + (опционально) POSIX Shared Memory для горячих потоков/снапшотов.
- **Persistency**: MVP — файловые снапшоты + журнал; далее — SQLite (в том же процессе `daemon`).
- **Интеграция с GitHub**: libcurl (REST/GraphQL), rate-limit aware, условные запросы (ETag/If-None-Match), экспоненциальный backoff.

### Структура репозитория

```text
GitFlowDashboard/
├─ daemon/                 # C (C11)
│  ├─ core/                # ядро: события, очередь, таймеры, воркеры
│  ├─ github/              # интеграция: REST/GraphQL, rate limiting, ETag
│  ├─ ipc/                 # Unix socket протокол, (опц.) POSIX shm
│  ├─ http/                # HTTP/1.1 + WebSocket сервер
│  ├─ storage/             # снапшоты (JSON), WAL; позже SQLite
│  ├─ metrics/             # внутренние метрики, /metrics (опц. Prometheus)
│  ├─ config/              # загрузка конфигов (TOML/JSON)
│  ├─ logging/             # syslog/файлы, уровни логирования
│  └─ CMakeLists.txt|Makefile
├─ tui/                    # C + ncurses
│  ├─ ui/                  # экраны, виджеты, темы
│  ├─ client/              # IPC-клиент, подписки, парсинг
│  └─ Makefile
├─ web/                    # TypeScript/React (Vite)
│  ├─ src/
│  │  ├─ app/              # роуты, layout
│  │  ├─ components/       # UI-компоненты
│  │  ├─ charts/           # диаграммы Chart.js/D3
│  │  ├─ hooks/            # data fetching, WebSocket
│  │  ├─ services/         # HTTP/WebSocket клиенты
│  │  ├─ types/            # TS-интерфейсы (генерация из схем)
│  │  └─ state/            # Zustand/Redux (если нужно)
│  ├─ public/
│  └─ vite.config.ts
├─ shared/                 # общие схемы и контракты
│  ├─ schema/              # JSON Schema для сущностей и событий
│  ├─ protocol/            # IPC/WS message spec (JSON)
│  └─ codegen/             # генерация C structs / TS interfaces
├─ scripts/                # утилиты, линтеры, генераторы
├─ infra/                  # Docker, systemd, compose, k8s (позже)
│  ├─ docker/
│  └─ systemd/
├─ tests/
│  ├─ daemon/              # cmocka/Unity
│  ├─ web/                 # Vitest/Playwright
│  └─ integration/         # мок GitHub, контрактные тесты
└─ README.md
```

### Модель данных (ядро)

- **Сущности**:
  - **User**: `id`, `login`, `avatarUrl`, `teamIds`.
  - **Repository**: `id`, `name`, `owner`, `defaultBranch`.
  - **PullRequest**: `id`, `number`, `repoId`, `authorId`, `state`, `labels`, `reviewers`, `updatedAt`.
  - **Issue**: `id`, `number`, `repoId`, `authorId`, `state`, `labels`, `updatedAt`.
  - **Review**: `id`, `prId`, `authorId`, `state`, `submittedAt`.
  - **ActivityEvent**: унифицированная лента (PR/Issue/Review/Comment/Push).
  - **MetricsSnapshot**: агрегаты по пользователю/команде/репозиторию, streaks/achievements.
- **Представления**:
  - Снапшоты для быстрой отрисовки (TUI/Web).
  - Инкрементальные события для реалтайм-обновлений.

### IPC-протокол (локальный, Unix socket)

- **Транспорт**: Unix Domain Socket, framed JSON (длина + JSON) для устойчивости к частичному чтению.
- **Команды**:
  - `GET_STATE { scope, filters }` → снапшот.
  - `SUBSCRIBE { streams: ["events","metrics"], filters }` → stream событий.
  - `PING`/`PONG`, `UNSUBSCRIBE`, `BYE`.
- **События**:
  - `state_update` (дифф или версия), `activity_event`, `metrics_update`, `daemon_health`.
- **SHM (опционально)**: кольцевой буфер только для чтения под горячие стримы; синхронизация через sequence number; JSON-указатели или бинарный layout (этап 2+).

Пример сообщения (концептуально):

```json
{
  "type": "SUBSCRIBE",
  "id": "cli-123",
  "streams": ["events", "metrics"],
  "filters": { "repos": ["owner/repo"], "authors": ["me"] }
}
```

### HTTP/WebSocket API (для Web)

- **HTTP**:
  - `GET /api/v1/state?scope=team|user|repo&...` — снапшот.
  - `GET /api/v1/metrics?range=7d&...` — агрегаты.
  - `GET /api/v1/config` — конфиг UI/фичи.
- **WebSocket**:
  - Каналы: `events`, `metrics`, `alerts`.
  - Протокол сообщений — тот же JSON (выравнивание с IPC).
  - Подписки/фильтры на соединение.
- **Контроль версий**: `X-API-Version`, `schemaVersion`.

### Внутренности `daemon`

- **Потоки**:
  - `fetcher`: опрашивает GitHub (pull), применяет ETag/If-None-Match, джиттер.
  - `aggregator`: нормализует, агрегирует, обновляет снапшоты.
  - `http_server`: обрабатывает REST/WS, push событий подписчикам.
  - `ipc_server`: обслуживает локальные клиенты (TUI).
  - `storage_flusher`: периодические снапшоты/WAL/SQLite.
  - `metrics_collector`: внутренние метрики/здоровье.
- **Очереди**: lock-free MPSC (если рационально) или mutex+condvar, backpressure.
- **Кэш**: in-memory LRU по ключам GitHub запросов (TTL + ETag).
- **Конфиг**: `config.toml` — токен, трекинг-правила, интервалы, фильтры.
- **Логи**: syslog + rotating file, уровни, трассировка запросов (корреляция).

### TUI (ncurses)

- **Архитектура**:
  - Модель: локальный снапшот состояния + подписка на события.
  - Контроллер: обработка клавиш, фильтры, поиск.
  - Представление: экраны `Dashboard`, `PRs`, `Issues`, `Activity`, `Goals`.
- **Обновления**: двойная буферизация, частичные перерисовки, debounce 16–33ms.

### Web (React)

- **Стек**: Vite, React, Zustand/Redux (минимально), React Query для данных.
- **Слои**:
  - `services/http.ts` и `services/ws.ts` (автореконнект, backoff, replay).
  - `types/*` — генерируются из `shared/schema`.
  - `charts/*` — обертки над Chart.js/D3, единый стиль.
- **UX**:
  - Быстрый first paint (<2s), code-splitting, skeletons, optimistic updates.
  - Доступность (ARIA), responsive.

### Хранение и кэширование

- **MVP**: `state.json` (снапшот), `events.log` (WAL). Ротация, контроль размера.
- **Phase 2**: SQLite (`state`, `events`, индексы по repo/user/time).
- **Кэш GitHub**: ответы + ETag; при 304 используем кэш; дедупликация по id.

### Конфигурация и секреты

- **Секреты**: `GITHUB_TOKEN` из окружения или `systemd` секрет-хранилище. Не отдавать в Web.
- **Конфиг**: `config.toml` (repos, users, teams, интервалы, лимиты).
- **Профили**: `dev`, `prod` (уровни логов, частоты опроса).

### Сборка и инструменты

- **C**: GCC/Clang, `Makefile` или CMake, `pkg-config` (ncurses, libcurl), `-fsanitize=address/ub`.
- **Web**: Node 20+, Vite, ESLint, Prettier, TypeScript strict.
- **Codegen**: скрипт генерации C/TS типов из JSON Schema (shared).

### Тестирование

- **Юнит (C)**: cmocka/Unity — парсинг, очереди, агрегация, протокол.
- **Интеграция**: мок GitHub (HTTP server), контрактные тесты API/WS.
- **Web**: Vitest/RTL, Playwright для e2e.
- **Нагрузка**: имитатор событий (10+ WS клиентов, burst).

### Наблюдаемость

- **Метрики**: `/metrics` (req/sec, latency p95, events/sec, queue depth, mem).
- **Трейсинг**: опционально OpenTelemetry (локально логами).
- **Health**: `/healthz` (live/ready), статус rate limit.

### Безопасность

- **HTTP**: без токена на клиенте; C-сервер — единственная точка GitHub.
- **CORS**: ограниченный origin, gzip, TLS (за обратным прокси).
- **IPC**: сокет в `$XDG_RUNTIME_DIR`, права 600, идентификация по uid.
- **Защита**: валидация JSON, лимиты сообщений, timeouts, DoS защита.

### Деплой

- **systemd**: `gitflowd.service` (Env `GITHUB_TOKEN`, `ProtectSystem=full`, `Restart=on-failure`).
- **Docker**: образ для `daemon` + отдельный для `web` (статический хостинг).
- **Compose**: локально `daemon + reverse-proxy + web`.
- **Prod**: обратный прокси (nginx/caddy), TLS, лог-агент.

### Производительность и лимиты

- **Цели**:
  - TUI обновления ≤100ms после события.
  - Web FCP <2s, интерактив <3s.
  - Память `daemon` <100MB.
  - ≥10 одновременных WS клиентов.
- **Тактика**:
  - Дифф-события вместо полных снапшотов.
  - Батчинг запросов GitHub, ETag/If-None-Match, приоритет очередей.
  - Предвычисление агрегатов, памяти — арена/пулы, zero-copy где возможно.

### Риски и смягчение

- **Rate limits**: агрессивное кэширование, backoff, усреднение интервалов, GraphQL выборка.
- **Утечки памяти**: sanitizer’ы, Valgrind в CI, пулы объектов.
- **Сложность протоколов**: единые схемы, автоген типов, контрактные тесты.
- **UI дрожание**: дебаунс, стабильные ключи, приоритетные обновления.

### Фазы разработки (с привязкой к архитектуре)

- **Phase 1 (MVP)**:
  - `daemon`: fetcher + aggregator + storage(JSON) + IPC(SUBSCRIBE/GET_STATE).
  - `tui`: базовая панель, список PR/Issues, live события.
- **Phase 2 (Full)**:
  - `daemon`: HTTP + WebSocket, кэш/ETag, SQLite, codegen типов.
  - `web`: SPA, графики, realtime.
- **Phase 3 (Advanced)**:
  - Командная аналитика, уведомления, оптимизации, деплой-автоматизация.

### Критические интерфейсы (контракты, без реализации)

- **Событие активностей (stream)**:

```json
{
  "type": "activity_event",
  "version": 1,
  "event": {
    "id": "evt_123",
    "kind": "pr_review_submitted",
    "repo": "owner/repo",
    "actor": "user",
    "targetId": "pr_42",
    "ts": "2026-01-06T12:00:00Z",
    "meta": { "state": "APPROVED" }
  }
}
```

- **Снапшот метрик**:

```json
{
  "type": "metrics_update",
  "version": 1,
  "scope": { "kind": "user", "id": "user" },
  "range": "7d",
  "metrics": {
    "prsOpened": 12,
    "reviewsGiven": 27,
    "issuesClosed": 5,
    "streakDays": 9,
    "achievements": ["FirstReview", "SevenDayStreak"]
  }
}
```

- **HTTP пример**: `GET /api/v1/state?scope=user&id=user&range=24h` → объект состояния (по схемам в `shared/schema`).

---

- Если хотите, могу сразу сгенерировать первые JSON Schema для `shared/schema` и скелет README с этими контрактами — без кода.

- Готов уточнить детали (набор метрик, экраны TUI/Web, формат ачивок) перед началом реализации.

- **Куда смотреть дальше**: определить минимальный набор сущностей/метрик для MVP и финализировать схемы сообщений в `shared/schema` (это ускорит обе стороны — TUI и Web).

- Коротко:
  - Единое C-ядро (`daemon`) с кэшом и реалтайм-паблишингом.
  - Локальный IPC для TUI, HTTP/WS для Web, общий JSON-контракт со схемами и автогеном типов.
  - MVP — файлы; далее — SQLite, кэш ETag и прод-наблюдаемость.

```plaintext
GitFlowDashboard/
├─ daemon/                 # C (C11)
│  ├─ core/                # ядро: события, очередь, таймеры, воркеры
│  ├─ github/              # интеграция: REST/GraphQL, rate limiting, ETag
│  ├─ ipc/                 # Unix socket протокол, (опц.) POSIX shm
│  ├─ http/                # HTTP/1.1 + WebSocket сервер
│  ├─ storage/             # снапшоты (JSON), WAL; позже SQLite
│  ├─ metrics/             # внутренние метрики, /metrics (опц. Prometheus)
│  ├─ config/              # загрузка конфигов (TOML/JSON)
│  ├─ logging/             # syslog/файлы, уровни логирования
│  └─ CMakeLists.txt|Makefile
├─ tui/                    # C + ncurses
│  ├─ ui/                  # экраны, виджеты, темы
│  ├─ client/              # IPC-клиент, подписки, парсинг
│  └─ Makefile
├─ web/                    # TypeScript/React (Vite)
│  ├─ src/
│  │  ├─ app/              # роуты, layout
│  │  ├─ components/       # UI-компоненты
│  │  ├─ charts/           # диаграммы Chart.js/D3
│  │  ├─ hooks/            # data fetching, WebSocket
│  │  ├─ services/         # HTTP/WebSocket клиенты
│  │  ├─ types/            # TS-интерфейсы (генерация из схем)
│  │  └─ state/            # Zustand/Redux (если нужно)
│  ├─ public/
│  └─ vite.config.ts
├─ shared/                 # общие схемы и контракты
│  ├─ schema/              # JSON Schema для сущностей и событий
│  ├─ protocol/            # IPC/WS message spec (JSON)
│  └─ codegen/             # генерация C structs / TS interfaces
├─ scripts/                # утилиты, линтеры, генераторы
├─ infra/                  # Docker, systemd, compose, k8s (позже)
│  ├─ docker/
│  └─ systemd/
├─ tests/
│  ├─ daemon/              # cmocka/Unity
│  ├─ web/                 # Vitest/Playwright
│  └─ integration/         # мок GitHub, контрактные тесты
└─ README.md
```

```json
{
  "type": "SUBSCRIBE",
  "id": "cli-123",
  "streams": ["events", "metrics"],
  "filters": { "repos": ["owner/repo"], "authors": ["me"] }
}
```

```json
{
  "type": "activity_event",
  "version": 1,
  "event": {
    "id": "evt_123",
    "kind": "pr_review_submitted",
    "repo": "owner/repo",
    "actor": "user",
    "targetId": "pr_42",
    "ts": "2026-01-06T12:00:00Z",
    "meta": { "state": "APPROVED" }
  }
}
```

```json
{
  "type": "metrics_update",
  "version": 1,
  "scope": { "kind": "user", "id": "user" },
  "range": "7d",
  "metrics": {
    "prsOpened": 12,
    "reviewsGiven": 27,
    "issuesClosed": 5,
    "streakDays": 9,
    "achievements": ["FirstReview", "SevenDayStreak"]
  }
}
```

---

**User**

создай GitFlowDashboard/
├─ daemon/                 # C (C11)
│  ├─ core/                # ядро: события, очередь, таймеры, воркеры
│  ├─ github/              # интеграция: REST/GraphQL, rate limiting, ETag
│  ├─ ipc/                 # Unix socket протокол, (опц.) POSIX shm
│  ├─ http/                # HTTP/1.1 + WebSocket сервер
│  ├─ storage/             # снапшоты (JSON), WAL; позже SQLite
│  ├─ metrics/             # внутренние метрики, /metrics (опц. Prometheus)
│  ├─ config/              # загрузка конфигов (TOML/JSON)
│  ├─ logging/             # syslog/файлы, уровни логирования
│  └─ CMakeLists.txt|Makefile
├─ tui/                    # C + ncurses
│  ├─ ui/                  # экраны, виджеты, темы
│  ├─ client/              # IPC-клиент, подписки, парсинг
│  └─ Makefile
├─ web/                    # TypeScript/React (Vite)
│  ├─ src/
│  │  ├─ app/              # роуты, layout
│  │  ├─ components/       # UI-компоненты
│  │  ├─ charts/           # диаграммы Chart.js/D3
│  │  ├─ hooks/            # data fetching, WebSocket
│  │  ├─ services/         # HTTP/WebSocket клиенты
│  │  ├─ types/            # TS-интерфейсы (генерация из схем)
│  │  └─ state/            # Zustand/Redux (если нужно)
│  ├─ public/
│  └─ vite.config.ts
├─ shared/                 # общие схемы и контракты
│  ├─ schema/              # JSON Schema для сущностей и событий
│  ├─ protocol/            # IPC/WS message spec (JSON)
│  └─ codegen/             # генерация C structs / TS interfaces
├─ scripts/                # утилиты, линтеры, генераторы
├─ infra/                  # Docker, systemd, compose, k8s (позже)
│  ├─ docker/
│  └─ systemd/
├─ tests/
│  ├─ daemon/              # cmocka/Unity
│  ├─ web/                 # Vitest/Playwright
│  └─ integration/         # мок GitHub, контрактные тесты
└─ README.md

---

**Cursor**

I'll scaffold the requested project directory structure with placeholder files, then show the resulting layout to confirm.

Готово. Создал структуру каталогов и плейсхолдеры файлов без реализации.

Создано:
- Директории: `daemon/*`, `tui/*`, `web/src/*`, `shared/*`, `scripts`, `infra/{docker,systemd}`, `tests/{daemon,web,integration}`
- Файлы-плейсхолдеры: `daemon/CMakeLists.txt`, `daemon/Makefile`, `tui/Makefile`, `web/vite.config.ts`, `README.md`
- В пустые директории добавлены `.gitkeep`

Текущее дерево (первые 3 уровня):

```
/home/mukadeshinigami/Code/GitFlowDashboard
├── daemon
│   ├── CMakeLists.txt
│   ├── Makefile
│   ├── config
│   ├── core
│   ├── github
│   ├── http
│   ├── ipc
│   ├── logging
│   ├── metrics
│   └── storage
├── tui
│   ├── Makefile
│   ├── client
│   └── ui
├── web
│   ├── public
│   ├── src
│   │   ├── app
│   │   ├── charts
│   │   ├── components
│   │   ├── hooks
│   │   ├── services
│   │   ├── state
│   │   └── types
│   └── vite.config.ts
├── shared
│   ├── codegen
│   ├── protocol
│   └── schema
├── scripts
├── infra
│   ├── docker
│   └── systemd
├── tests
│   ├── daemon
│   ├── integration
│   └── web
└── README.md
```

- Скажи, используем `CMake` или `Make` в `daemon` по умолчанию — оставлю только один файл и допишу базовый билд-скелет. 
- Могу также добавить базовый `package.json` для `web` и `eslint/prettier` конфиги — без реализации.

```plaintext
/home/mukadeshinigami/Code/GitFlowDashboard
├── daemon
│   ├── CMakeLists.txt
│   ├── Makefile
│   ├── config
│   ├── core
│   ├── github
│   ├── http
│   ├── ipc
│   ├── logging
│   ├── metrics
│   └── storage
├── tui
│   ├── Makefile
│   ├── client
│   └── ui
├── web
│   ├── public
│   ├── src
│   │   ├── app
│   │   ├── charts
│   │   ├── components
│   │   ├── hooks
│   │   ├── services
│   │   ├── state
│   │   └── types
│   └── vite.config.ts
├── shared
│   ├── codegen
│   ├── protocol
│   └── schema
├── scripts
├── infra
│   ├── docker
│   └── systemd
├── tests
│   ├── daemon
│   ├── integration
│   └── web
└── README.md
```
