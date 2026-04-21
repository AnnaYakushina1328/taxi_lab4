# Домашнее задание 04: Проектирование и работа с MongoDB

## Вариант 16 — Система заказа такси

---

## Описание проекта

В рамках лабораторной работы реализовано проектирование документной модели MongoDB для системы заказа такси на **Yandex userver**.

Проект продолжает предыдущую лабораторную работу и сохраняет существующий REST API сервис на **C++ / userver** с подключением к **PostgreSQL**. В рамках лабораторной работы 04 в проект дополнительно добавлена **MongoDB** как вторая база данных.

В проекте выполнены:

- проектирование документной модели MongoDB;
- выбор коллекций и структуры документов;
- обоснование использования embedded documents и references;
- заполнение MongoDB тестовыми данными;
- подготовка MongoDB-запросов для операций варианта;
- настройка `$jsonSchema` валидации;
- запуск API, PostgreSQL и MongoDB через Docker Compose.

---

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

---

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

### Назначение основных файлов

- `schema.sql` — SQL-схема PostgreSQL из предыдущей лабораторной работы;
- `data.sql` — тестовые данные PostgreSQL;
- `queries.sql` — SQL-запросы для PostgreSQL;
- `optimization.md` — описание индексов и оптимизации PostgreSQL-запросов;
- `schema_design.md` — описание документной модели MongoDB;
- `data.js` — заполнение MongoDB тестовыми данными;
- `queries.js` — MongoDB-запросы для операций варианта;
- `validation.js` — настройка и проверка `$jsonSchema`;
- `openapi.yaml` — OpenAPI спецификация REST API;
- `docker-compose.yaml` — запуск PostgreSQL, MongoDB и API сервиса;
- `src/storage` — слой доступа к данным сервиса;
- `src/handlers` — реализация HTTP endpoint-ов;
- `src/middlewares` — middleware для Bearer-аутентификации.

---

## Документная модель MongoDB

Для MongoDB выбраны три основные коллекции:

- `users`
- `drivers`
- `rides`

Такое разбиение соответствует предметной области сервиса такси и хорошо подходит для выполнения операций варианта 16.

Использована гибридная модель:

- основные сущности вынесены в отдельные коллекции;
- небольшие связанные данные хранятся как embedded documents;
- связи между поездками, пользователями и водителями сохраняются через references.

### Коллекция `users`

Коллекция `users` хранит пользователей сервиса такси.

Основные поля:

- `_id` — идентификатор;
- `login` — логин;
- `password` — пароль;
- `first_name` — имя;
- `last_name` — фамилия;
- `created_at` — дата создания;
- `status` — состояние пользователя;
- `profile` — вложенный объект с контактными данными.

`profile` хранится как embedded document, потому что тесно связан с пользователем, имеет небольшой размер и не требует отдельного жизненного цикла.

### Коллекция `drivers`

Коллекция `drivers` хранит сведения о водителях.

Основные поля:

- `_id` — идентификатор;
- `login` — логин водителя;
- `first_name` — имя;
- `last_name` — фамилия;
- `status` — статус водителя;
- `registered_at` — дата регистрации;
- `car` — вложенный объект с автомобилем;
- `profile` — вложенный объект с контактной информацией и номером удостоверения.

Объекты `car` и `profile` хранятся как embedded documents, потому что читаются вместе с водителем и не используются как отдельные сущности.

### Коллекция `rides`

Коллекция `rides` хранит поездки.

Основные поля:

- `_id` — идентификатор поездки;
- `user_id` — ссылка на пользователя;
- `driver_id` — ссылка на водителя;
- `status` — статус поездки;
- `created_at` — дата создания;
- `accepted_at` — дата принятия поездки;
- `completed_at` — дата завершения;
- `route` — адрес отправления и адрес назначения;
- `fare` — стоимость поездки;
- `user_snapshot` — краткие данные пользователя;
- `driver_snapshot` — краткие данные водителя;
- `events` — список событий по поездке.

Коллекция `rides` выделена отдельно, потому что поездка является самостоятельной сущностью с собственным жизненным циклом.

### Выбор между embedded и references

**Embedded documents** используются для:

- `users.profile`
- `drivers.car`
- `drivers.profile`
- `rides.route`
- `rides.fare`
- `rides.user_snapshot`
- `rides.driver_snapshot`
- `rides.events`

**References** используются для:

- `rides.user_id -> users._id`
- `rides.driver_id -> drivers._id`

Такой выбор позволяет:

- не раздувать документы пользователей и водителей историей поездок;
- удобно строить запросы по активным заказам и истории поездок;
- при этом хранить часто используемые короткие данные прямо в документе поездки.

Подробное описание модели приведено в файле `schema_design.md`.

---

## Тестовые данные MongoDB

Для MongoDB подготовлен файл `data.js`.

В нём:

- создаются коллекции `users`, `drivers`, `rides`;
- удаляются старые данные перед повторной загрузкой;
- добавляется минимум 10 документов в каждую коллекцию;
- используются разные типы MongoDB данных:
  - `String`
  - `Boolean`
  - `Date`
  - `Array`
  - `Object`
  - `ObjectId`

Проверка после загрузки:

- `users = 10`
- `drivers = 10`
- `rides = 10`

---

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

В `validation.js` показаны два сценария:

- вставка корректного документа;
- попытка вставки некорректного документа, которая завершается ошибкой `Document failed validation`.

---

## MongoDB-запросы

Основные MongoDB-запросы собраны в файле `queries.js`.

В запросах используются:

- `insertOne`
- `findOne`
- `find`
- `updateOne`
- `deleteOne`
- `aggregate`

Также используются операторы:

- `$eq`
- `$ne`
- `$gt`
- `$lt`
- `$in`
- `$and`
- `$or`
- `$push`

---

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

```http
HTTP/1.1 200 OK
```

---

## Подключение к базам данных

### PostgreSQL

Подключение к PostgreSQL внутри контейнера:

```bash
docker exec -it taxi-postgres psql -U taxi -d taxi
```

Проверка таблиц:

```sql
\dt
```

### MongoDB

Подключение к MongoDB внутри контейнера:

```bash
docker exec -it taxi-mongo mongosh
```

Проверка MongoDB:

```javascript
db.adminCommand({ ping: 1 })
```

---

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

---

## Примеры работы с API

### Создание пользователя

```bash
curl -i -X POST http://localhost:8080/users \
  -H "Content-Type: application/json" \
  -d '{
    "login": "test.user",
    "password": "pass123",
    "full_name": "Test User"
  }'
```

### Логин

```bash
curl -i -X POST http://localhost:8080/auth/login \
  -H "Content-Type: application/json" \
  -d '{
    "login": "test.user",
    "password": "pass123"
  }'
```

### Регистрация водителя

```bash
curl -i -X POST http://localhost:8080/drivers \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer TOKEN_HERE" \
  -d '{
    "user_id": 11,
    "car_model": "Toyota Camry",
    "car_number": "K123KK77",
    "license_number": "LIC-1011"
  }'
```

### Создание поездки

```bash
curl -i -X POST http://localhost:8080/rides \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer TOKEN_HERE" \
  -d '{
    "passenger_id": 11,
    "pickup_address": "Lenina 1",
    "destination_address": "Tverskaya 10"
  }'
```

### Получение активных поездок

```bash
curl -i "http://localhost:8080/rides?status=active" \
  -H "Authorization: Bearer TOKEN_HERE"
```

### Получение истории поездок пользователя

```bash
curl -i "http://localhost:8080/rides?user_id=11" \
  -H "Authorization: Bearer TOKEN_HERE"
```

### Принятие поездки водителем

```bash
curl -i -X PATCH http://localhost:8080/rides/11/accept \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer TOKEN_HERE" \
  -d '{
    "driver_id": 11
  }'
```

### Завершение поездки

```bash
curl -i -X PATCH http://localhost:8080/rides/11/complete \
  -H "Authorization: Bearer TOKEN_HERE"
```

---

## Файлы лабораторной работы 04

В репозитории присутствуют основные файлы лабораторной работы 04:

- `schema_design.md`
- `data.js`
- `queries.js`
- `validation.js`
- `README.md`
- `Dockerfile`
- `docker-compose.yaml`

Также в проекте сохранены файлы предыдущей лабораторной работы с PostgreSQL:

- `schema.sql`
- `data.sql`
- `queries.sql`
- `optimization.md`

---

## Что изменено по сравнению с лабораторной работой 03

В лабораторной работе 03 сервис был построен на PostgreSQL.

В лабораторной работе 04:

- в проект добавлена MongoDB как вторая база данных;
- спроектирована документная модель для сущностей `users`, `drivers`, `rides`;
- подготовлены тестовые данные MongoDB;
- подготовлены MongoDB-запросы для операций варианта;
- настроена валидация через `$jsonSchema`;
- MongoDB подключена к API на userver;
- добавлены отдельные HTTP endpoint-ы для работы с MongoDB-пользователями;
- запуск проекта расширен до PostgreSQL + MongoDB + API.

---

## Ограничения текущей реализации

- MongoDB в текущем состоянии используется как дополнительная база данных;
- основная логика API по-прежнему ориентирована на PostgreSQL из предыдущей лабораторной работы;
- пароли в тестовых данных хранятся в открытом виде для учебной демонстрации;
- не реализованы платежи, рейтинги и отзывы;
- проект ориентирован на учебную демонстрацию документной модели и MongoDB-запросов.

---

## MongoDB endpoint-ы в API

Для демонстрации подключения MongoDB к сервису на userver в проект добавлены отдельные endpoint-ы:

- `POST /mongo/users` — создание пользователя в MongoDB
- `GET /mongo/users?login=...` — поиск пользователя по логину в MongoDB
- `GET /mongo/users?name_mask=...` — поиск пользователей по маске имени или фамилии в MongoDB

### Примеры запросов к MongoDB endpoint-ам

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

## Вывод

В рамках лабораторной работы была спроектирована документная модель MongoDB для системы заказа такси. В проект была добавлена вторая база данных без отказа от PostgreSQL, подготовлены тестовые данные, запросы и схема валидации, а также сохранён простой запуск через Docker Compose.

В результате получен учебный проект, который:

- запускается через Docker Compose;
- использует PostgreSQL и MongoDB в одном сервисе;
- содержит документную модель для варианта 16;
- включает MongoDB-скрипты, документацию и примеры запуска.

