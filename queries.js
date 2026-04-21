db = db.getSiblingDB('taxi_mongo_db');

const tempUserId = ObjectId("674444444444444444444441");
const tempDriverId = ObjectId("675555555555555555555551");
const tempRideId = ObjectId("676666666666666666666661");

print("========================================");
print("MongoDB queries for lab4, variant 16");
print("========================================");

/*
  Очистка временных документов,
  чтобы скрипт можно было запускать много раз.
*/
db.rides.deleteOne({ _id: tempRideId });
db.drivers.deleteOne({ _id: tempDriverId });
db.users.deleteOne({ _id: tempUserId });

print("\n1. CREATE: создание нового пользователя");
const createUserResult = db.users.insertOne({
  _id: tempUserId,
  login: "mongo.test.user",
  password: "hashed_password_temp",
  first_name: "Тест",
  last_name: "Пассажир",
  created_at: new Date(),
  status: "active",
  profile: {
    email: "mongo.test.user@example.com",
    phone: "+79995554433"
  }
});
printjson(createUserResult);

print("\n2. READ: поиск пользователя по логину через $eq");
const findUserByLogin = db.users.findOne({
  login: { $eq: "mongo.test.user" }
});
printjson(findUserByLogin);

print("\n3. READ: поиск пользователя по маске имени и фамилии через $or и regex");
const findUsersByMask = db.users.find({
  $or: [
    { first_name: { $regex: "ив", $options: "i" } },
    { last_name: { $regex: "ов", $options: "i" } }
  ]
}).toArray();
printjson(findUsersByMask);

print("\n4. CREATE: регистрация водителя");
const createDriverResult = db.drivers.insertOne({
  _id: tempDriverId,
  login: "mongo.test.driver",
  first_name: "Тест",
  last_name: "Водитель",
  status: "available",
  registered_at: new Date(),
  car: {
    brand: "Toyota",
    model: "Camry",
    plate_number: "K123KK777",
    color: "black"
  },
  profile: {
    phone: "+79994443322",
    license_number: "77AB999999"
  }
});
printjson(createDriverResult);

print("\n5. READ: выборка водителей по статусу через $in");
const findAvailableDrivers = db.drivers.find({
  status: { $in: ["available", "busy"] }
}).limit(5).toArray();
printjson(findAvailableDrivers);

print("\n6. CREATE: создание заказа поездки");
const createRideResult = db.rides.insertOne({
  _id: tempRideId,
  user_id: tempUserId,
  driver_id: null,
  status: "created",
  created_at: new Date(),
  accepted_at: null,
  completed_at: null,
  route: {
    pickup_address: "Москва, Ленинградский проспект, 1",
    dropoff_address: "Москва, Тверская, 10"
  },
  fare: {
    amount: 650,
    currency: "RUB"
  },
  user_snapshot: {
    login: "mongo.test.user",
    first_name: "Тест",
    last_name: "Пассажир"
  },
  driver_snapshot: null,
  events: [
    {
      type: "created",
      at: new Date()
    }
  ]
});
printjson(createRideResult);

print("\n7. READ: получение активных заказов через $in");
const activeRides = db.rides.find({
  status: { $in: ["created", "accepted"] }
}).toArray();
printjson(activeRides);

print("\n8. READ: пример сложной выборки поездок через $and, $gt, $lt, $ne");
const complexRideSearch = db.rides.find({
  $and: [
    { "fare.amount": { $gt: 300 } },
    { "fare.amount": { $lt: 1000 } },
    { status: { $ne: "cancelled" } }
  ]
}).toArray();
printjson(complexRideSearch);

print("\n9. UPDATE: принятие заказа водителем");
const driverForAccept = db.drivers.findOne({
  _id: tempDriverId,
  status: { $eq: "available" }
});

if (driverForAccept) {
  const acceptTime = new Date();

  const acceptRideResult = db.rides.updateOne(
    {
      _id: tempRideId,
      status: "created"
    },
    {
      $set: {
        driver_id: driverForAccept._id,
        status: "accepted",
        accepted_at: acceptTime,
        driver_snapshot: {
          login: driverForAccept.login,
          first_name: driverForAccept.first_name,
          last_name: driverForAccept.last_name,
          car_brand: driverForAccept.car.brand,
          car_model: driverForAccept.car.model,
          plate_number: driverForAccept.car.plate_number
        }
      },
      $push: {
        events: {
          type: "accepted",
          at: acceptTime
        }
      }
    }
  );
  printjson(acceptRideResult);

  const updateDriverStatus = db.drivers.updateOne(
    { _id: driverForAccept._id },
    { $set: { status: "busy" } }
  );
  print("\nОбновление статуса водителя:");
  printjson(updateDriverStatus);
} else {
  print("Нет доступного водителя для принятия заказа");
}

print("\n10. READ: получение активных заказов после принятия");
const activeRidesAfterAccept = db.rides.find({
  status: { $in: ["created", "accepted"] }
}).toArray();
printjson(activeRidesAfterAccept);

print("\n11. READ: получение истории поездок пользователя");
const rideHistoryByUser = db.rides.find({
  user_id: tempUserId
}).sort({ created_at: -1 }).toArray();
printjson(rideHistoryByUser);

print("\n12. READ: история поездок пользователя через aggregation pipeline");
const rideHistoryPipeline = db.rides.aggregate([
  {
    $match: {
      user_id: tempUserId
    }
  },
  {
    $project: {
      _id: 1,
      status: 1,
      created_at: 1,
      accepted_at: 1,
      completed_at: 1,
      pickup_address: "$route.pickup_address",
      dropoff_address: "$route.dropoff_address",
      fare_amount: "$fare.amount",
      driver_login: "$driver_snapshot.login",
      driver_name: {
        $concat: [
          { $ifNull: ["$driver_snapshot.first_name", ""] },
          " ",
          { $ifNull: ["$driver_snapshot.last_name", ""] }
        ]
      }
    }
  },
  {
    $sort: { created_at: -1 }
  }
]).toArray();
printjson(rideHistoryPipeline);

print("\n13. UPDATE: завершение поездки");
const completeTime = new Date();

const completeRideResult = db.rides.updateOne(
  {
    _id: tempRideId,
    status: "accepted"
  },
  {
    $set: {
      status: "completed",
      completed_at: completeTime
    },
    $push: {
      events: {
        type: "completed",
        at: completeTime
      }
    }
  }
);
printjson(completeRideResult);

const releaseDriverResult = db.drivers.updateOne(
  { _id: tempDriverId },
  { $set: { status: "available" } }
);
print("\nВозврат статуса водителя в available:");
printjson(releaseDriverResult);

print("\n14. READ: проверка завершённой поездки через $and");
const completedRideCheck = db.rides.findOne({
  $and: [
    { _id: tempRideId },
    { status: "completed" }
  ]
});
printjson(completedRideCheck);

print("\n15. READ: выборка поездок по типам событий через $in");
const ridesWithSpecificEvents = db.rides.find({
  "events.type": { $in: ["created", "accepted", "completed"] }
}).limit(5).toArray();
printjson(ridesWithSpecificEvents);

print("\n16. DELETE: удаление тестовой поездки");
const deleteRideResult = db.rides.deleteOne({
  _id: tempRideId
});
printjson(deleteRideResult);

print("\n17. DELETE: удаление тестового водителя");
const deleteDriverResult = db.drivers.deleteOne({
  _id: tempDriverId
});
printjson(deleteDriverResult);

print("\n18. DELETE: удаление тестового пользователя");
const deleteUserResult = db.users.deleteOne({
  _id: tempUserId
});
printjson(deleteUserResult);

print("\n19. Финальная проверка количества документов");
print("users=" + db.users.countDocuments());
print("drivers=" + db.drivers.countDocuments());
print("rides=" + db.rides.countDocuments());
