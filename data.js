db = db.getSiblingDB('taxi_mongo_db');

db.users.drop();
db.drivers.drop();
db.rides.drop();

const userIds = {
  u1: ObjectId("671111111111111111111111"),
  u2: ObjectId("671111111111111111111112"),
  u3: ObjectId("671111111111111111111113"),
  u4: ObjectId("671111111111111111111114"),
  u5: ObjectId("671111111111111111111115"),
  u6: ObjectId("671111111111111111111116"),
  u7: ObjectId("671111111111111111111117"),
  u8: ObjectId("671111111111111111111118"),
  u9: ObjectId("671111111111111111111119"),
  u10: ObjectId("67111111111111111111111a")
};

const driverIds = {
  d1: ObjectId("672222222222222222222221"),
  d2: ObjectId("672222222222222222222222"),
  d3: ObjectId("672222222222222222222223"),
  d4: ObjectId("672222222222222222222224"),
  d5: ObjectId("672222222222222222222225"),
  d6: ObjectId("672222222222222222222226"),
  d7: ObjectId("672222222222222222222227"),
  d8: ObjectId("672222222222222222222228"),
  d9: ObjectId("672222222222222222222229"),
  d10: ObjectId("67222222222222222222222a")
};

const rideIds = {
  r1: ObjectId("673333333333333333333331"),
  r2: ObjectId("673333333333333333333332"),
  r3: ObjectId("673333333333333333333333"),
  r4: ObjectId("673333333333333333333334"),
  r5: ObjectId("673333333333333333333335"),
  r6: ObjectId("673333333333333333333336"),
  r7: ObjectId("673333333333333333333337"),
  r8: ObjectId("673333333333333333333338"),
  r9: ObjectId("673333333333333333333339"),
  r10: ObjectId("67333333333333333333333a")
};

db.createCollection("users");
db.createCollection("drivers");
db.createCollection("rides");

db.users.insertMany([
  {
    _id: userIds.u1,
    login: "ivan.petrov",
    password: "hashed_password_01",
    first_name: "Иван",
    last_name: "Петров",
    created_at: ISODate("2026-04-01T09:00:00Z"),
    status: "active",
    profile: { email: "ivan.petrov@example.com", phone: "+79990000001" }
  },
  {
    _id: userIds.u2,
    login: "anna.smirnova",
    password: "hashed_password_02",
    first_name: "Анна",
    last_name: "Смирнова",
    created_at: ISODate("2026-04-02T09:10:00Z"),
    status: "active",
    profile: { email: "anna.smirnova@example.com", phone: "+79990000002" }
  },
  {
    _id: userIds.u3,
    login: "pavel.ivanov",
    password: "hashed_password_03",
    first_name: "Павел",
    last_name: "Иванов",
    created_at: ISODate("2026-04-03T09:20:00Z"),
    status: "active",
    profile: { email: "pavel.ivanov@example.com", phone: "+79990000003" }
  },
  {
    _id: userIds.u4,
    login: "elena.sokolova",
    password: "hashed_password_04",
    first_name: "Елена",
    last_name: "Соколова",
    created_at: ISODate("2026-04-04T09:30:00Z"),
    status: "active",
    profile: { email: "elena.sokolova@example.com", phone: "+79990000004" }
  },
  {
    _id: userIds.u5,
    login: "dmitry.kozin",
    password: "hashed_password_05",
    first_name: "Дмитрий",
    last_name: "Козин",
    created_at: ISODate("2026-04-05T09:40:00Z"),
    status: "active",
    profile: { email: "dmitry.kozin@example.com", phone: "+79990000005" }
  },
  {
    _id: userIds.u6,
    login: "olga.morozova",
    password: "hashed_password_06",
    first_name: "Ольга",
    last_name: "Морозова",
    created_at: ISODate("2026-04-06T09:50:00Z"),
    status: "active",
    profile: { email: "olga.morozova@example.com", phone: "+79990000006" }
  },
  {
    _id: userIds.u7,
    login: "sergey.volkov",
    password: "hashed_password_07",
    first_name: "Сергей",
    last_name: "Волков",
    created_at: ISODate("2026-04-07T10:00:00Z"),
    status: "active",
    profile: { email: "sergey.volkov@example.com", phone: "+79990000007" }
  },
  {
    _id: userIds.u8,
    login: "maria.fedorova",
    password: "hashed_password_08",
    first_name: "Мария",
    last_name: "Федорова",
    created_at: ISODate("2026-04-08T10:10:00Z"),
    status: "active",
    profile: { email: "maria.fedorova@example.com", phone: "+79990000008" }
  },
  {
    _id: userIds.u9,
    login: "nikita.orlov",
    password: "hashed_password_09",
    first_name: "Никита",
    last_name: "Орлов",
    created_at: ISODate("2026-04-09T10:20:00Z"),
    status: "blocked",
    profile: { email: "nikita.orlov@example.com", phone: "+79990000009" }
  },
  {
    _id: userIds.u10,
    login: "irina.popova",
    password: "hashed_password_10",
    first_name: "Ирина",
    last_name: "Попова",
    created_at: ISODate("2026-04-10T10:30:00Z"),
    status: "active",
    profile: { email: "irina.popova@example.com", phone: "+79990000010" }
  }
]);

db.drivers.insertMany([
  {
    _id: driverIds.d1,
    login: "driver.alex",
    first_name: "Алексей",
    last_name: "Сидоров",
    status: "available",
    registered_at: ISODate("2026-04-01T11:00:00Z"),
    car: { brand: "Kia", model: "Rio", plate_number: "A123BC777", color: "white" },
    profile: { phone: "+79990000101", license_number: "77AB123456" }
  },
  {
    _id: driverIds.d2,
    login: "driver.max",
    first_name: "Максим",
    last_name: "Ильин",
    status: "available",
    registered_at: ISODate("2026-04-02T11:10:00Z"),
    car: { brand: "Hyundai", model: "Solaris", plate_number: "B234CD777", color: "black" },
    profile: { phone: "+79990000102", license_number: "77AB123457" }
  },
  {
    _id: driverIds.d3,
    login: "driver.roman",
    first_name: "Роман",
    last_name: "Киселев",
    status: "busy",
    registered_at: ISODate("2026-04-03T11:20:00Z"),
    car: { brand: "Volkswagen", model: "Polo", plate_number: "C345DE777", color: "gray" },
    profile: { phone: "+79990000103", license_number: "77AB123458" }
  },
  {
    _id: driverIds.d4,
    login: "driver.anton",
    first_name: "Антон",
    last_name: "Серов",
    status: "available",
    registered_at: ISODate("2026-04-04T11:30:00Z"),
    car: { brand: "Skoda", model: "Rapid", plate_number: "D456EF777", color: "silver" },
    profile: { phone: "+79990000104", license_number: "77AB123459" }
  },
  {
    _id: driverIds.d5,
    login: "driver.oleg",
    first_name: "Олег",
    last_name: "Фомин",
    status: "offline",
    registered_at: ISODate("2026-04-05T11:40:00Z"),
    car: { brand: "Toyota", model: "Camry", plate_number: "E567FG777", color: "black" },
    profile: { phone: "+79990000105", license_number: "77AB123460" }
  },
  {
    _id: driverIds.d6,
    login: "driver.ivan",
    first_name: "Иван",
    last_name: "Нестеров",
    status: "available",
    registered_at: ISODate("2026-04-06T11:50:00Z"),
    car: { brand: "Renault", model: "Logan", plate_number: "F678GH777", color: "blue" },
    profile: { phone: "+79990000106", license_number: "77AB123461" }
  },
  {
    _id: driverIds.d7,
    login: "driver.petr",
    first_name: "Петр",
    last_name: "Логинов",
    status: "busy",
    registered_at: ISODate("2026-04-07T12:00:00Z"),
    car: { brand: "Lada", model: "Vesta", plate_number: "G789HI777", color: "white" },
    profile: { phone: "+79990000107", license_number: "77AB123462" }
  },
  {
    _id: driverIds.d8,
    login: "driver.egor",
    first_name: "Егор",
    last_name: "Мельников",
    status: "available",
    registered_at: ISODate("2026-04-08T12:10:00Z"),
    car: { brand: "Nissan", model: "Almera", plate_number: "H890IJ777", color: "red" },
    profile: { phone: "+79990000108", license_number: "77AB123463" }
  },
  {
    _id: driverIds.d9,
    login: "driver.artem",
    first_name: "Артем",
    last_name: "Титов",
    status: "available",
    registered_at: ISODate("2026-04-09T12:20:00Z"),
    car: { brand: "Ford", model: "Focus", plate_number: "I901JK777", color: "gray" },
    profile: { phone: "+79990000109", license_number: "77AB123464" }
  },
  {
    _id: driverIds.d10,
    login: "driver.kirill",
    first_name: "Кирилл",
    last_name: "Зайцев",
    status: "offline",
    registered_at: ISODate("2026-04-10T12:30:00Z"),
    car: { brand: "Chevrolet", model: "Aveo", plate_number: "J012KL777", color: "yellow" },
    profile: { phone: "+79990000110", license_number: "77AB123465" }
  }
]);

db.rides.insertMany([
  {
    _id: rideIds.r1,
    user_id: userIds.u1,
    driver_id: driverIds.d1,
    status: "completed",
    created_at: ISODate("2026-04-11T08:00:00Z"),
    accepted_at: ISODate("2026-04-11T08:02:00Z"),
    completed_at: ISODate("2026-04-11T08:35:00Z"),
    route: {
      pickup_address: "Москва, Тверская, 1",
      dropoff_address: "Москва, Арбат, 10"
    },
    fare: { amount: 540, currency: "RUB" },
    user_snapshot: { login: "ivan.petrov", first_name: "Иван", last_name: "Петров" },
    driver_snapshot: {
      login: "driver.alex",
      first_name: "Алексей",
      last_name: "Сидоров",
      car_brand: "Kia",
      car_model: "Rio",
      plate_number: "A123BC777"
    },
    events: [
      { type: "created", at: ISODate("2026-04-11T08:00:00Z") },
      { type: "accepted", at: ISODate("2026-04-11T08:02:00Z") },
      { type: "completed", at: ISODate("2026-04-11T08:35:00Z") }
    ]
  },
  {
    _id: rideIds.r2,
    user_id: userIds.u2,
    driver_id: driverIds.d2,
    status: "accepted",
    created_at: ISODate("2026-04-11T09:00:00Z"),
    accepted_at: ISODate("2026-04-11T09:03:00Z"),
    completed_at: null,
    route: {
      pickup_address: "Москва, Ленинградский проспект, 15",
      dropoff_address: "Москва, Белорусская площадь, 2"
    },
    fare: { amount: 610, currency: "RUB" },
    user_snapshot: { login: "anna.smirnova", first_name: "Анна", last_name: "Смирнова" },
    driver_snapshot: {
      login: "driver.max",
      first_name: "Максим",
      last_name: "Ильин",
      car_brand: "Hyundai",
      car_model: "Solaris",
      plate_number: "B234CD777"
    },
    events: [
      { type: "created", at: ISODate("2026-04-11T09:00:00Z") },
      { type: "accepted", at: ISODate("2026-04-11T09:03:00Z") }
    ]
  },
  {
    _id: rideIds.r3,
    user_id: userIds.u3,
    driver_id: driverIds.d3,
    status: "completed",
    created_at: ISODate("2026-04-11T10:00:00Z"),
    accepted_at: ISODate("2026-04-11T10:04:00Z"),
    completed_at: ISODate("2026-04-11T10:41:00Z"),
    route: {
      pickup_address: "Москва, Проспект Мира, 8",
      dropoff_address: "Москва, Сретенка, 21"
    },
    fare: { amount: 720, currency: "RUB" },
    user_snapshot: { login: "pavel.ivanov", first_name: "Павел", last_name: "Иванов" },
    driver_snapshot: {
      login: "driver.roman",
      first_name: "Роман",
      last_name: "Киселев",
      car_brand: "Volkswagen",
      car_model: "Polo",
      plate_number: "C345DE777"
    },
    events: [
      { type: "created", at: ISODate("2026-04-11T10:00:00Z") },
      { type: "accepted", at: ISODate("2026-04-11T10:04:00Z") },
      { type: "completed", at: ISODate("2026-04-11T10:41:00Z") }
    ]
  },
  {
    _id: rideIds.r4,
    user_id: userIds.u4,
    driver_id: null,
    status: "created",
    created_at: ISODate("2026-04-11T11:00:00Z"),
    accepted_at: null,
    completed_at: null,
    route: {
      pickup_address: "Москва, Кутузовский проспект, 4",
      dropoff_address: "Москва, Киевская площадь, 1"
    },
    fare: { amount: 480, currency: "RUB" },
    user_snapshot: { login: "elena.sokolova", first_name: "Елена", last_name: "Соколова" },
    driver_snapshot: null,
    events: [
      { type: "created", at: ISODate("2026-04-11T11:00:00Z") }
    ]
  },
  {
    _id: rideIds.r5,
    user_id: userIds.u5,
    driver_id: driverIds.d4,
    status: "accepted",
    created_at: ISODate("2026-04-11T12:00:00Z"),
    accepted_at: ISODate("2026-04-11T12:02:00Z"),
    completed_at: null,
    route: {
      pickup_address: "Москва, Новослободская, 12",
      dropoff_address: "Москва, Павелецкая площадь, 3"
    },
    fare: { amount: 670, currency: "RUB" },
    user_snapshot: { login: "dmitry.kozin", first_name: "Дмитрий", last_name: "Козин" },
    driver_snapshot: {
      login: "driver.anton",
      first_name: "Антон",
      last_name: "Серов",
      car_brand: "Skoda",
      car_model: "Rapid",
      plate_number: "D456EF777"
    },
    events: [
      { type: "created", at: ISODate("2026-04-11T12:00:00Z") },
      { type: "accepted", at: ISODate("2026-04-11T12:02:00Z") }
    ]
  },
  {
    _id: rideIds.r6,
    user_id: userIds.u6,
    driver_id: driverIds.d6,
    status: "completed",
    created_at: ISODate("2026-04-11T13:00:00Z"),
    accepted_at: ISODate("2026-04-11T13:05:00Z"),
    completed_at: ISODate("2026-04-11T13:32:00Z"),
    route: {
      pickup_address: "Москва, Таганская, 5",
      dropoff_address: "Москва, Курская, 7"
    },
    fare: { amount: 390, currency: "RUB" },
    user_snapshot: { login: "olga.morozova", first_name: "Ольга", last_name: "Морозова" },
    driver_snapshot: {
      login: "driver.ivan",
      first_name: "Иван",
      last_name: "Нестеров",
      car_brand: "Renault",
      car_model: "Logan",
      plate_number: "F678GH777"
    },
    events: [
      { type: "created", at: ISODate("2026-04-11T13:00:00Z") },
      { type: "accepted", at: ISODate("2026-04-11T13:05:00Z") },
      { type: "completed", at: ISODate("2026-04-11T13:32:00Z") }
    ]
  },
  {
    _id: rideIds.r7,
    user_id: userIds.u7,
    driver_id: driverIds.d7,
    status: "accepted",
    created_at: ISODate("2026-04-11T14:00:00Z"),
    accepted_at: ISODate("2026-04-11T14:01:00Z"),
    completed_at: null,
    route: {
      pickup_address: "Москва, Сокол, 2",
      dropoff_address: "Москва, Динамо, 18"
    },
    fare: { amount: 430, currency: "RUB" },
    user_snapshot: { login: "sergey.volkov", first_name: "Сергей", last_name: "Волков" },
    driver_snapshot: {
      login: "driver.petr",
      first_name: "Петр",
      last_name: "Логинов",
      car_brand: "Lada",
      car_model: "Vesta",
      plate_number: "G789HI777"
    },
    events: [
      { type: "created", at: ISODate("2026-04-11T14:00:00Z") },
      { type: "accepted", at: ISODate("2026-04-11T14:01:00Z") }
    ]
  },
  {
    _id: rideIds.r8,
    user_id: userIds.u8,
    driver_id: null,
    status: "created",
    created_at: ISODate("2026-04-11T15:00:00Z"),
    accepted_at: null,
    completed_at: null,
    route: {
      pickup_address: "Москва, Бауманская, 11",
      dropoff_address: "Москва, Электрозаводская, 9"
    },
    fare: { amount: 350, currency: "RUB" },
    user_snapshot: { login: "maria.fedorova", first_name: "Мария", last_name: "Федорова" },
    driver_snapshot: null,
    events: [
      { type: "created", at: ISODate("2026-04-11T15:00:00Z") }
    ]
  },
  {
    _id: rideIds.r9,
    user_id: userIds.u9,
    driver_id: driverIds.d8,
    status: "completed",
    created_at: ISODate("2026-04-11T16:00:00Z"),
    accepted_at: ISODate("2026-04-11T16:04:00Z"),
    completed_at: ISODate("2026-04-11T16:38:00Z"),
    route: {
      pickup_address: "Москва, Савеловская, 6",
      dropoff_address: "Москва, Менделеевская, 3"
    },
    fare: { amount: 560, currency: "RUB" },
    user_snapshot: { login: "nikita.orlov", first_name: "Никита", last_name: "Орлов" },
    driver_snapshot: {
      login: "driver.egor",
      first_name: "Егор",
      last_name: "Мельников",
      car_brand: "Nissan",
      car_model: "Almera",
      plate_number: "H890IJ777"
    },
    events: [
      { type: "created", at: ISODate("2026-04-11T16:00:00Z") },
      { type: "accepted", at: ISODate("2026-04-11T16:04:00Z") },
      { type: "completed", at: ISODate("2026-04-11T16:38:00Z") }
    ]
  },
  {
    _id: rideIds.r10,
    user_id: userIds.u10,
    driver_id: driverIds.d9,
    status: "accepted",
    created_at: ISODate("2026-04-11T17:00:00Z"),
    accepted_at: ISODate("2026-04-11T17:03:00Z"),
    completed_at: null,
    route: {
      pickup_address: "Москва, Парк Победы, 4",
      dropoff_address: "Москва, Университет, 12"
    },
    fare: { amount: 790, currency: "RUB" },
    user_snapshot: { login: "irina.popova", first_name: "Ирина", last_name: "Попова" },
    driver_snapshot: {
      login: "driver.artem",
      first_name: "Артем",
      last_name: "Титов",
      car_brand: "Ford",
      car_model: "Focus",
      plate_number: "I901JK777"
    },
    events: [
      { type: "created", at: ISODate("2026-04-11T17:00:00Z") },
      { type: "accepted", at: ISODate("2026-04-11T17:03:00Z") }
    ]
  }
]);

print("Users count: " + db.users.countDocuments());
print("Drivers count: " + db.drivers.countDocuments());
print("Rides count: " + db.rides.countDocuments());
