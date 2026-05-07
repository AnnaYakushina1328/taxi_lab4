# Домашнее задание 05: Оптимизация производительности через кеширование и rate limiting

Вариант 16: Система заказа такси.

## Описание проекта

Проект реализует REST API сервиса заказа такси.

Основные сущности:

- пользователь;
- водитель;
- поездка.

Основные операции:

- создание нового пользователя;
- поиск пользователя по логину;
- поиск пользователя по маске имени и фамилии;
- регистрация водителя;
- создание заказа поездки;
- получение активных заказов;
- принятие заказа водителем;
- получение истории поездок пользователя;
- завершение поездки.

В рамках пятой лабораторной работы добавлены:

- кеширование часто вызываемых GET endpoint-ов;
- инвалидация кеша при изменении данных;
- rate limiting для endpoint-а получения активных заказов;
- документация по производительности в файле performance_design.md.

## Кеширование

Кеширование применяется к endpoint-ам:

- GET /users?login=...
- GET /users?name_mask=...
- GET /rides?status=active
- GET /rides?user_id=...

Используется стратегия Cache-Aside / Lazy Loading.

В ответ добавляется заголовок:

- X-Cache: MISS — данные получены из БД;
- X-Cache: HIT — данные получены из кеша.

## Инвалидация кеша

Инвалидация выполняется при изменении данных:

- после POST /users очищается кеш пользователей;
- после POST /drivers очищается кеш водителей;
- после POST /rides очищается кеш поездок;
- после PATCH /rides/{id}/accept очищается кеш поездок;
- после PATCH /rides/{id}/complete очищается кеш поездок.

## Rate limiting

Rate limiting реализован для endpoint-а:

- GET /rides?status=active

Алгоритм:

- Fixed Window Counter

Лимит:

- 100 запросов в минуту.

При превышении лимита сервис возвращает:

- HTTP 429 Too Many Requests

Также возвращаются заголовки:

- X-RateLimit-Limit
- X-RateLimit-Remaining
- X-RateLimit-Reset
- Retry-After

## Структура проекта

Основные файлы пятой лабораторной работы:

- performance_design.md — описание стратегии кеширования и rate limiting;
- src/performance/simple_performance.hpp — реализация кеша и rate limiter;
- src/handlers/users_get.cpp — кеширование поиска пользователей;
- src/handlers/rides_get.cpp — кеширование поездок и rate limiting;
- src/handlers/users_create.cpp — инвалидация кеша пользователей;
- src/handlers/drivers_create.cpp — инвалидация кеша водителей;
- src/handlers/rides_create.cpp — инвалидация кеша поездок;
- src/handlers/rides_accept.cpp — инвалидация кеша поездок после принятия заказа;
- src/handlers/rides_complete.cpp — инвалидация кеша поездок после завершения заказа;
- Dockerfile — сборка приложения;
- docker-compose.yaml — запуск приложения, PostgreSQL и MongoDB.

## Запуск проекта

Склонировать репозиторий:

    git clone ССЫЛКА_НА_РЕПОЗИТОРИЙ
    cd НАЗВАНИЕ_ПАПКИ

Запустить проект:

    docker compose up --build -d

Проверить контейнеры:

    docker compose ps

Остановить проект:

    docker compose down -v

## Получение токена для проверки

API защищён авторизацией, поэтому перед проверками нужно получить токен.

Создать тестового пользователя:

    curl -s -X POST "http://localhost:8080/users" -H "Content-Type: application/json" -d '{"login":"cache_test_user","password":"pass123","full_name":"Cache Test User"}'

Получить токен:

    TOKEN=$(curl -s -X POST "http://localhost:8080/auth/login" -H "Content-Type: application/json" -d '{"login":"cache_test_user","password":"pass123"}' | python3 -c 'import sys,json; print(json.load(sys.stdin).get("token",""))')

Проверить, что токен получен:

    echo $TOKEN

Дальше во всех защищённых запросах используется заголовок:

    Authorization: Bearer $TOKEN

## Проверка кеширования активных поездок

Первый запрос должен вернуть X-Cache: MISS:

    curl -s -D - -o /tmp/rides_1.json -H "Authorization: Bearer $TOKEN" "http://localhost:8080/rides?status=active" | grep -Ei "HTTP/|X-Cache|X-RateLimit"
    cat /tmp/rides_1.json

Повторный такой же запрос должен вернуть X-Cache: HIT:

    curl -s -D - -o /tmp/rides_2.json -H "Authorization: Bearer $TOKEN" "http://localhost:8080/rides?status=active" | grep -Ei "HTTP/|X-Cache|X-RateLimit"
    cat /tmp/rides_2.json

Ожидаемый результат:

    X-Cache: MISS
    X-Cache: HIT

Также для endpoint-а GET /rides?status=active должны возвращаться заголовки rate limiting:

    X-RateLimit-Limit
    X-RateLimit-Remaining
    X-RateLimit-Reset

## Проверка кеширования истории поездок пользователя

Первый запрос должен вернуть X-Cache: MISS:

    curl -s -D - -o /tmp/rides_history_1.json -H "Authorization: Bearer $TOKEN" "http://localhost:8080/rides?user_id=1" | grep -Ei "HTTP/|X-Cache"
    cat /tmp/rides_history_1.json

Повторный такой же запрос должен вернуть X-Cache: HIT:

    curl -s -D - -o /tmp/rides_history_2.json -H "Authorization: Bearer $TOKEN" "http://localhost:8080/rides?user_id=1" | grep -Ei "HTTP/|X-Cache"
    cat /tmp/rides_history_2.json

Ожидаемый результат:

    X-Cache: MISS
    X-Cache: HIT

## Проверка кеширования пользователей

Первый запрос должен вернуть X-Cache: MISS:

    curl -s -D - -o /tmp/users_1.json -H "Authorization: Bearer $TOKEN" "http://localhost:8080/users?name_mask=Cache" | grep -Ei "HTTP/|X-Cache"
    cat /tmp/users_1.json

Повторный такой же запрос должен вернуть X-Cache: HIT:

    curl -s -D - -o /tmp/users_2.json -H "Authorization: Bearer $TOKEN" "http://localhost:8080/users?name_mask=Cache" | grep -Ei "HTTP/|X-Cache"
    cat /tmp/users_2.json

Ожидаемый результат:

    X-Cache: MISS
    X-Cache: HIT

## Проверка rate limiting

Для endpoint-а GET /rides?status=active установлен лимит 100 запросов в минуту.

Перед проверкой можно подождать минуту, чтобы лимит был чистым:

    sleep 65

Проверка:

    for i in $(seq 1 105); do curl -s -o /dev/null -w "$i -> %{http_code}\n" -H "Authorization: Bearer $TOKEN" "http://localhost:8080/rides?status=active"; done

В начале должны возвращаться ответы 200, а после превышения лимита — 429.

Пример ожидаемого результата:

    1 -> 200
    2 -> 200
    ...
    100 -> 200
    101 -> 429
    102 -> 429

После превышения лимита можно отдельно посмотреть заголовки ответа:

    curl -i -H "Authorization: Bearer $TOKEN" "http://localhost:8080/rides?status=active"

Ожидаемые заголовки:

    HTTP/1.1 429 Too Many Requests
    X-RateLimit-Limit: 100
    X-RateLimit-Remaining: 0
    X-RateLimit-Reset: ...
    Retry-After: 60

Если при проверке сразу возвращается 429, значит лимит уже был израсходован в текущем минутном окне. Нужно подождать около минуты и повторить запрос.

## Документация по производительности

Подробное описание стратегии кеширования, rate limiting, hot paths, TTL, инвалидации кеша и метрик мониторинга находится в файле:

- performance_design.md
