# 🚀 Быстрая шпаргалка по C

## Указатели

```c
int x = 10;
int* ptr = &x;      // указатель на x
int value = *ptr;   // разыменование (получаем 10)
*ptr = 20;          // изменяем x через указатель
```

## Память

```c
// Выделение
void* ptr = malloc(size);           // не инициализирована
void* ptr = calloc(count, size);    // обнулена
void* ptr = realloc(old_ptr, size); // изменение размера

// Освобождение
free(ptr);
ptr = NULL;  // хорошая практика
```

## Строки

```c
char str[] = "Hello";           // массив (можно изменять)
char* ptr = "Hello";            // указатель на литерал (нельзя изменять!)
char* copy = strdup("Hello");   // копия (нужно free!)

// Длина
size_t len = strlen(str);

// Копирование
strcpy(dest, src);              // копирует (опасно!)
strncpy(dest, src, n);          // копирует n символов
snprintf(dest, size, "%s", src); // безопасное форматирование
```

## Структуры

```c
// Определение
typedef struct {
    char* name;
    int age;
} Person;

// Использование
Person p;              // на стеке
p.name = strdup("Иван");
p.age = 25;

Person* ptr = malloc(sizeof(Person));  // в куче
ptr->name = strdup("Иван");            // -> для указателей
ptr->age = 25;
free(ptr->name);
free(ptr);
```

## Массивы

```c
// Статический
int arr[10];

// Динамический
int* arr = malloc(sizeof(int) * 10);
arr[0] = 1;
free(arr);

// Массив структур
Person* people = malloc(sizeof(Person) * 10);
people[0].name = strdup("Иван");
free(people[0].name);
free(people);
```

## Файлы

```c
FILE* file = fopen("file.txt", "r");
if (file == NULL) {
    // ошибка
    return;
}

// Размер файла
fseek(file, 0, SEEK_END);
long size = ftell(file);
fseek(file, 0, SEEK_SET);

// Чтение
char* buffer = malloc(size + 1);
fread(buffer, 1, size, file);
buffer[size] = '\0';

fclose(file);
```

## Переменные окружения

```c
const char* token = getenv("GITHUB_TOKEN");
if (token != NULL) {
    printf("Token: %s\n", token);
}
```

## Проверки

```c
// Проверка указателя
if (ptr == NULL) {
    return NULL;
}

// Проверка результата malloc
void* ptr = malloc(size);
if (ptr == NULL) {
    // ошибка выделения памяти
    return NULL;
}
```

## Паттерны

### Выделение и освобождение

```c
// Выделение
char* str = strdup("Hello");
if (str == NULL) {
    return NULL;
}

// Использование
printf("%s\n", str);

// Освобождение
free(str);
str = NULL;
```

### Обработка ошибок

```c
int* data = malloc(100);
if (data == NULL) {
    fprintf(stderr, "Ошибка выделения памяти\n");
    return -1;
}

// ... работа ...

free(data);
return 0;
```

### Инициализация структуры

```c
Config* config = calloc(1, sizeof(Config));
if (config == NULL) {
    return NULL;
}

// Или вручную
config->github_token = NULL;
config->repositories = NULL;
config->repositories_count = 0;
```

## Полезные функции

```c
// Память
void* malloc(size_t size);
void* calloc(size_t count, size_t size);
void* realloc(void* ptr, size_t size);
void free(void* ptr);

// Строки
size_t strlen(const char* str);
char* strcpy(char* dest, const char* src);
char* strdup(const char* str);  // нужно free!
int snprintf(char* str, size_t size, const char* format, ...);

// Файлы
FILE* fopen(const char* path, const char* mode);
int fclose(FILE* file);
size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
int fseek(FILE* stream, long offset, int whence);
long ftell(FILE* stream);

// Окружение
char* getenv(const char* name);
```

## Важные правила

1. ✅ Каждому `malloc`/`calloc`/`strdup` соответствует `free`
2. ✅ Всегда проверяйте результат `malloc` на `NULL`
3. ✅ После `free()` устанавливайте указатель в `NULL`
4. ✅ Используйте `snprintf` вместо `sprintf`
5. ✅ Проверяйте указатели перед использованием
6. ✅ Закрывайте файлы через `fclose()`

## Отладка

```c
// Компиляция с отладочной информацией
gcc -g -Wall -Wextra program.c -o program

// Запуск с AddressSanitizer
gcc -fsanitize=address program.c -o program
./program

// Valgrind
valgrind --leak-check=full ./program
```



