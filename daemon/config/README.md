# Конфигурация демона

## Как работает загрузка токена

### Механизм работы

1. **`.env` файл** содержит токен:
   ```
   GITHUB_TOKEN=ghp_your_token_here
   ```

2. **`scripts/load-env.sh`** загружает переменные в окружение:
   ```bash
   export GITHUB_TOKEN="ghp_your_token_here"
   ```

3. **`config.json`** содержит пустой токен:
   ```json
   {
       "github": {
           "token": "",  // ← пустая строка = использовать из окружения
           ...
       }
   }
   ```

4. **C-код демона** при запуске:
   - Читает `config.json` → видит пустой `token: ""`
   - Проверяет переменную окружения `GITHUB_TOKEN` (через `getenv()`)
   - Использует токен из окружения

### Использование

#### Вариант 1: Загрузка через скрипт
```bash
# Загрузить переменные из .env
source scripts/load-env.sh

# Запустить демон (в той же сессии shell)
./daemon/gitflowd
```

#### Вариант 2: Экспорт вручную
```bash
export GITHUB_TOKEN="your_token_here"
./daemon/gitflowd
```

#### Вариант 3: В systemd service
```ini
[Service]
EnvironmentFile=/path/to/GitFlowDashboard/.env
ExecStart=/path/to/daemon/gitflowd
```

### Приоритет токена

1. **Высший приоритет**: `GITHUB_TOKEN` (переменная окружения)
2. **Низший приоритет**: `token` из `config.json`

Если токен найден в переменной окружения, он используется, даже если в `config.json` есть значение.

### Безопасность

- ✅ `.env` файл в `.gitignore` (не коммитится)
- ✅ `.env.example` - шаблон (можно коммитить)
- ✅ Токен никогда не хранится в `config.json` (пустая строка)
- ✅ Токен читается только из переменных окружения или `.env`





