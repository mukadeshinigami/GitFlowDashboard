# Script to load environment variables from .env file
# Usage: source scripts/load-env.sh

# Получаем путь к директории скрипта
if [ -n "${BASH_SOURCE[0]}" ]; then
    # Когда скрипт вызывается через source
    SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
else
    # Fallback на $0, если BASH_SOURCE не доступен
    SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
fi

PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
ENV_FILE="$PROJECT_ROOT/.env"

if [ -f "$ENV_FILE" ]; then
    set -a  # экспортировать все переменные
    source "$ENV_FILE"
    set +a
    echo "✓ Переменные окружения загружены из .env"
else
    echo "⚠ Файл .env не найден: $ENV_FILE"
    exit 1
fi

