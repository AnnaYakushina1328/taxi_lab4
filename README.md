# Лабораторная работа 5. Кеширование и rate limiting в сервисе такси

Вариант 16: система заказа такси.

## Что делает проект

Это учебный REST API сервис для заказа такси.

В проекте есть три основные сущности:

- пользователь;
- водитель;
- поездка.

По API можно создать пользователя, найти пользователя по логину или имени, зарегистрировать водителя, создать заказ поездки, получить активные заказы, принять поездку водителем, посмотреть историю поездок пользователя и завершить поездку.

В пятой лабораторной работе добавлены:

- кеширование часто повторяющихся GET-запросов;
- очистка кеша после изменения данных;
- ограничение частоты запросов для активных заказов;
- описание выбранного решения в файле `performance_design.md`.

## Важное замечание про авторизацию

Большая часть API защищена авторизацией. Если выполнять проверочные GET-запросы без токена, сервис вернёт:

    HTTP/1.1 401 Unauthorized

Поэтому перед проверкой кеширования и rate limiting нужно сначала создать тестового пользователя, получить токен через `/auth/login` и дальше передавать его в заголовке:

    Authorization: Bearer $TOKEN

Подробные команды для этого приведены ниже в разделе проверки.

## Что кешируется

Кеширование добавлено для endpoint-ов:

- `GET /users?login=...`
- `GET /users?name_mask=...`
- `GET /rides?status=active`
- `GET /rides?user_id=...`

Используется стратегия Cache-Aside. Сначала сервис смотрит, есть ли готовый ответ в кеше. Если есть, он сразу отдаёт его клиенту. Если нет, сервис идёт в PostgreSQL, формирует JSON-ответ и сохраняет его в кеш на короткое время.

Для проверки работы кеша в ответ добавлен заголовок:

- `X-Cache: MISS` — ответ был собран через запрос к базе;
- `X-Cache: HIT` — ответ был взят из кеша.

TTL:

- пользователи — 60 секунд;
- история поездок пользователя — 60 секунд;
- активные поездки — 15 секунд.

Для активных поездок TTL меньше, потому что список доступных заказов в такси быстро устаревает.

## Инвалидация кеша

Кеш очищается после операций, которые меняют данные:

- `POST /users` — очищается кеш пользователей;
- `POST /drivers` — очищается кеш водителей;
- `POST /rides` — очищается кеш поездок;
- `PATCH /rides/{id}/accept` — очищается кеш поездок;
- `PATCH /rides/{id}/complete` — очищается кеш поездок.

Например, если пассажир создал новую поездку, список активных заказов должен обновиться. Поэтому после создания поездки кеш с префиксом `rides:` очищается.

## Rate limiting

Rate limiting добавлен для endpoint-а:

- `GET /rides?status=active`

Этот endpoint выбран потому, что его могут часто вызывать водители, когда ищут доступные заказы.

Алгоритм:

- Fixed Window Counter.

Лимит:

- 100 запросов в минуту.

Если лимит превышен, сервис возвращает:

- `HTTP 429 Too Many Requests`.

Также в ответе есть заголовки:

- `X-RateLimit-Limit`;
- `X-RateLimit-Remaining`;
- `X-RateLimit-Reset`;
- `Retry-After`.

## Основные файлы

Файлы, которые относятся к пятой лабораторной:

- `performance_design.md` — описание решения по кешированию и rate limiting;
- `src/performance/simple_performance.hpp` — реализация кеша и rate limiter;
- `src/handlers/users_get.cpp` — кеширование поиска пользователей;
- `src/handlers/rides_get.cpp` — кеширование поездок и rate limiting;
- `src/handlers/users_create.cpp` — очистка кеша пользователей;
- `src/handlers/drivers_create.cpp` — очистка кеша водителей;
- `src/handlers/rides_create.cpp` — очистка кеша поездок;
- `src/handlers/rides_accept.cpp` — очистка кеша после принятия поездки;
- `src/handlers/rides_complete.cpp` — очистка кеша после завершения поездки;
- `Dockerfile` — сборка приложения;
- `docker-compose.yaml` — запуск приложения, PostgreSQL и MongoDB.

## Быстрый запуск для проверки

Склонировать репозиторий:

    git clone https://github.com/AnnaYakushina1328/taxi_lab5.git
    cd taxi_lab5

Запустить проект:

    docker compose up --build -d

Проверить контейнеры:

    docker compose ps

Нужно дождаться, чтобы сервисы были в состоянии `healthy`.

Остановить проект после проверки:

    docker compose down -v

## Получение токена для проверки

API защищён авторизацией, поэтому сначала нужно получить токен.

Создать тестового пользователя:

    curl -s -X POST "http://localhost:8080/users" -H "Content-Type: application/json" -d '{"login":"cache_test_user","password":"pass123","full_name":"Cache Test User"}'

Если пользователь уже существует, это не страшно. Можно сразу выполнить логин.

Получить токен:

    TOKEN=$(curl -s -X POST "http://localhost:8080/auth/login" -H "Content-Type: application/json" -d '{"login":"cache_test_user","password":"pass123"}' | python3 -c 'import sys,json; print(json.load(sys.stdin).get("token",""))')

Проверить, что токен получен:

    echo $TOKEN

Если команда вывела пустую строку, значит токен не получен и защищённые запросы вернут `401 Unauthorized`.

Дальше во всех проверочных запросах используется заголовок:

    -H "Authorization: Bearer $TOKEN"

## Проверка кеша на активных поездках

Первый запрос должен пойти в базу и вернуть `X-Cache: MISS`:

    curl -s -D - -o /tmp/rides_1.json -H "Authorization: Bearer $TOKEN" "http://localhost:8080/rides?status=active" | grep -Ei "HTTP/|X-Cache|X-RateLimit"
    cat /tmp/rides_1.json

Повторный такой же запрос должен вернуться из кеша и показать `X-Cache: HIT`:

    curl -s -D - -o /tmp/rides_2.json -H "Authorization: Bearer $TOKEN" "http://localhost:8080/rides?status=active" | grep -Ei "HTTP/|X-Cache|X-RateLimit"
    cat /tmp/rides_2.json

Ожидаемый результат:

    X-Cache: MISS
    X-Cache: HIT

У этого же endpoint-а должны быть заголовки rate limiting:

    X-RateLimit-Limit
    X-RateLimit-Remaining
    X-RateLimit-Reset

## Проверка кеша на истории поездок

Первый запрос:

    curl -s -D - -o /tmp/rides_history_1.json -H "Authorization: Bearer $TOKEN" "http://localhost:8080/rides?user_id=1" | grep -Ei "HTTP/|X-Cache"
    cat /tmp/rides_history_1.json

Повторный запрос:

    curl -s -D - -o /tmp/rides_history_2.json -H "Authorization: Bearer $TOKEN" "http://localhost:8080/rides?user_id=1" | grep -Ei "HTTP/|X-Cache"
    cat /tmp/rides_history_2.json

Ожидаемый результат:

    X-Cache: MISS
    X-Cache: HIT

Если сразу появился `X-Cache: HIT`, значит этот запрос уже выполнялся раньше и ответ ещё не успел устареть по TTL.

## Проверка кеша на пользователях

Первый запрос:

    curl -s -D - -o /tmp/users_1.json -H "Authorization: Bearer $TOKEN" "http://localhost:8080/users?name_mask=Cache" | grep -Ei "HTTP/|X-Cache"
    cat /tmp/users_1.json

Повторный запрос:

    curl -s -D - -o /tmp/users_2.json -H "Authorization: Bearer $TOKEN" "http://localhost:8080/users?name_mask=Cache" | grep -Ei "HTTP/|X-Cache"
    cat /tmp/users_2.json

Ожидаемый результат:

    X-Cache: MISS
    X-Cache: HIT

## Проверка rate limiting

Перед проверкой можно подождать минуту, чтобы счётчик запросов был чистым:

    sleep 65

Запустить 105 запросов подряд:

    for i in $(seq 1 105); do curl -s -o /dev/null -w "$i -> %{http_code}\n" -H "Authorization: Bearer $TOKEN" "http://localhost:8080/rides?status=active"; done

До сотого запроса должны возвращаться ответы `200`. После превышения лимита должны появиться `429`.

Пример:

    99 -> 200
    100 -> 200
    101 -> 429
    102 -> 429

Проверить заголовки после превышения лимита:

    curl -i -H "Authorization: Bearer $TOKEN" "http://localhost:8080/rides?status=active"

Ожидаемые заголовки:

    HTTP/1.1 429 Too Many Requests
    X-RateLimit-Limit: 100
    X-RateLimit-Remaining: 0
    X-RateLimit-Reset: ...
    Retry-After: 60

Если при проверке сразу возвращается `429`, значит лимит уже был израсходован в текущем минутном окне. Нужно подождать около минуты и повторить запрос.

## Короткая проверка одной командой

После запуска Docker и получения токена можно быстро проверить кеш активных поездок:

    curl -s -D - -o /tmp/rides_1.json -H "Authorization: Bearer $TOKEN" "http://localhost:8080/rides?status=active" | grep -Ei "HTTP/|X-Cache|X-RateLimit"
    curl -s -D - -o /tmp/rides_2.json -H "Authorization: Bearer $TOKEN" "http://localhost:8080/rides?status=active" | grep -Ei "HTTP/|X-Cache|X-RateLimit"

В выводе должно быть сначала `X-Cache: MISS`, потом `X-Cache: HIT`.

## Документация

Подробное описание выбора hot paths, TTL, инвалидации кеша и rate limiting находится в файле:

- `performance_design.md`


## MongoDB endpoint-ы для rides

Дополнительно в API добавлены endpoint-ы для работы с поездками в MongoDB:

- `POST /mongo/rides` — создание заказа поездки в MongoDB
- `GET /mongo/rides?status=active` — получение активных поездок из MongoDB
- `GET /mongo/rides?user_login=...` — получение истории поездок пользователя из MongoDB

```bash
curl -i -X POST http://localhost:8080/mongo/rides \
  -H "Content-Type: application/json" \
  -d '{
    "user_login": "mongo.api.user",
    "pickup_address": "Moscow, Tverskaya 1",
    "destination_address": "Moscow, Arbat 10",
    "amount": 650
  }'
```

```bash
curl -i "http://localhost:8080/mongo/rides?status=active"
```

```bash
curl -i "http://localhost:8080/mongo/rides?user_login=mongo.api.user"
```

