# Script to load environment variables from .env file
# Source scripts/load-env.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
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

