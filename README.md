# Домашнее задание 4: Проектирование и работа с MongoDB

## Вариант 16 — Система заказа такси

## Описание проекта

В рамках лабораторной работы реализовано проектирование документной модели MongoDB для системы заказа такси на Yandex userver.

Проект продолжает предыдущую лабораторную работу и сохраняет существующий REST API сервис на C++ / userver с подключением к PostgreSQL. В рамках лабораторной работы 4 в проект дополнительно добавлена MongoDB как вторая база данных.

В проекте выполнены:

- проектирование документной модели MongoDB;
- выбор коллекций и структуры документов;
- обоснование использования embedded documents и references;
- заполнение MongoDB тестовыми данными;
- подготовка MongoDB-запросов для операций варианта;
- настройка `$jsonSchema`-валидации;
- подключение MongoDB к API на userver;
- добавление Mongo endpoint-ов для `users` и `rides`;
- запуск API, PostgreSQL и MongoDB через Docker Compose.

## Функциональность варианта

Система содержит следующие основные сущности:

- `users` — пользователи;
- `drivers` — водители;
- `rides` — поездки.

Для варианта 16 предусмотрены следующие операции:

- создание нового пользователя;
- поиск пользователя по логину;
- поиск пользователя по маске имени и фамилии;
- регистрация водителя;
- создание заказа поездки;
- получение активных заказов;
- принятие заказа водителем;
- получение истории поездок пользователя;
- завершение поездки.

## Структура проекта

```text
taxi_lab4/
├── configs/
├── src/
│   ├── handlers/
│   ├── middlewares/
│   ├── models/
│   └── storage/
├── tests/
├── schema.sql
├── data.sql
├── queries.sql
├── optimization.md
├── schema_design.md
├── data.js
├── queries.js
├── validation.js
├── openapi.yaml
├── Dockerfile
├── docker-compose.yaml
├── CMakeLists.txt
└── README.md
```

## Документная модель MongoDB

Для MongoDB выбраны три основные коллекции:

- `users`;
- `drivers`;
- `rides`.

Использована гибридная модель:

- основные сущности вынесены в отдельные коллекции;
- небольшие связанные данные хранятся как embedded documents;
- связи между поездками, пользователями и водителями сохраняются через references.

## Коллекция `users`

Хранит пользователей сервиса такси.

Основные поля:

- `_id`;
- `login`;
- `password`;
- `first_name`;
- `last_name`;
- `created_at`;
- `status`;
- `profile`.

## Коллекция `drivers`

Хранит сведения о водителях.

Основные поля:

- `_id`;
- `login`;
- `first_name`;
- `last_name`;
- `status`;
- `registered_at`;
- `car`;
- `profile`.

## Коллекция `rides`

Хранит поездки.

Основные поля:

- `_id`;
- `user_id`;
- `driver_id`;
- `status`;
- `created_at`;
- `accepted_at`;
- `completed_at`;
- `route`;
- `fare`;
- `user_snapshot`;
- `driver_snapshot`;
- `events`.

Подробное описание документной модели приведено в `schema_design.md`.

## Тестовые данные MongoDB

Для MongoDB подготовлен файл `data.js`.

В нём:

- создаются коллекции `users`, `drivers`, `rides`;
- удаляются старые данные перед повторной загрузкой;
- добавляется минимум 10 документов в каждую коллекцию.

Проверка после загрузки:

```text
users=10
drivers=10
rides=10
```

## Валидация схемы

Для коллекции `users` настроена валидация через `$jsonSchema`.

Проверяются:

- обязательные поля;
- типы значений;
- допустимые значения `status`;
- формат `login`;
- длина строк;
- структура объекта `profile`;
- формат `email` и `phone`.

В `validation.js` показаны:

- вставка корректного документа;
- попытка вставки некорректного документа с ошибкой `Document failed validation`.

## MongoDB-запросы

Основные MongoDB-запросы собраны в `queries.js`.

В запросах используются:

- `insertOne`;
- `findOne`;
- `find`;
- `updateOne`;
- `deleteOne`;
- `aggregate`.

Также используются операторы:

- `$eq`;
- `$ne`;
- `$gt`;
- `$lt`;
- `$in`;
- `$and`;
- `$or`;
- `$push`.

## Запуск проекта

### 1. Клонирование репозитория

```bash
git clone https://github.com/AnnaYakushina1328/taxi_lab4.git
cd taxi_lab4
```

### 2. Запуск PostgreSQL, MongoDB и API

```bash
docker compose up -d --build
```

### 3. Проверка контейнеров

```bash
docker compose ps
```

### 4. Проверка доступности API

```bash
curl -i http://localhost:8080/ping
```

Ожидаемый ответ:

```text
HTTP/1.1 200 OK
```

## Работа с MongoDB

### Загрузка тестовых данных

```bash
docker exec -i taxi-mongo mongosh < data.js
```

### Проверка количества документов

```bash
docker exec -i taxi-mongo mongosh --quiet --eval '
db = db.getSiblingDB("taxi_mongo_db");
print("users=" + db.users.countDocuments());
print("drivers=" + db.drivers.countDocuments());
print("rides=" + db.rides.countDocuments());
'
```

### Запуск валидации схемы

```bash
docker exec -i taxi-mongo mongosh < validation.js
```

### Запуск MongoDB-запросов

```bash
docker exec -i taxi-mongo mongosh < queries.js
```

## MongoDB endpoint-ы в API

Для демонстрации подключения MongoDB к сервису на userver добавлены отдельные endpoint-ы.

### Users

- `POST /mongo/users` — создание пользователя в MongoDB;
- `GET /mongo/users?login=...` — поиск пользователя по логину в MongoDB;
- `GET /mongo/users?name_mask=...` — поиск пользователей по маске имени или фамилии в MongoDB.

Примеры:

```bash
curl -i -X POST http://localhost:8080/mongo/users \
  -H "Content-Type: application/json" \
  -d '{
    "login": "mongo.api.user",
    "password": "pass1234",
    "first_name": "Anna",
    "last_name": "Yakushina"
  }'
```

```bash
curl -i "http://localhost:8080/mongo/users?login=mongo.api.user"
```

```bash
curl -G -i --data-urlencode "name_mask=Ann" http://localhost:8080/mongo/users
```

### Rides

- `POST /mongo/rides` — создание заказа поездки в MongoDB;
- `GET /mongo/rides?status=active` — получение активных поездок из MongoDB;
- `GET /mongo/rides?user_login=...` — получение истории поездок пользователя из MongoDB.

Примеры:

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

## Вывод

В рамках лабораторной работы была спроектирована документная модель MongoDB для системы заказа такси. В проект добавлена вторая база данных без отказа от PostgreSQL, подготовлены тестовые данные, запросы, схема валидации и отдельные Mongo endpoint-ы для `users` и `rides`.

В результате получен проект, который:

- запускается через Docker Compose;
- использует PostgreSQL и MongoDB в одном сервисе;
- содержит документную модель для варианта 16;
- включает MongoDB-скрипты, документацию и примеры запуска.
