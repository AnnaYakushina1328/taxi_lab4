db = db.getSiblingDB('taxi_mongo_db');

const validationResult = db.runCommand({
  collMod: "users",
  validator: {
    $jsonSchema: {
      bsonType: "object",
      required: [
        "login",
        "password",
        "first_name",
        "last_name",
        "created_at",
        "status",
        "profile"
      ],
      additionalProperties: false,
      properties: {
        _id: {
          bsonType: "objectId",
          description: "_id должен быть ObjectId"
        },
        login: {
          bsonType: "string",
          minLength: 3,
          maxLength: 50,
          pattern: "^[a-z0-9._-]+$",
          description: "login должен быть строкой длиной 3-50 и содержать только a-z, 0-9, ., _, -"
        },
        password: {
          bsonType: "string",
          minLength: 6,
          description: "password должен быть строкой длиной не менее 6 символов"
        },
        first_name: {
          bsonType: "string",
          minLength: 1,
          maxLength: 100,
          description: "first_name должен быть непустой строкой"
        },
        last_name: {
          bsonType: "string",
          minLength: 1,
          maxLength: 100,
          description: "last_name должен быть непустой строкой"
        },
        created_at: {
          bsonType: "date",
          description: "created_at должен быть датой"
        },
        status: {
          enum: ["active", "blocked"],
          description: "status должен быть active или blocked"
        },
        profile: {
          bsonType: "object",
          required: ["email", "phone"],
          additionalProperties: false,
          properties: {
            email: {
              bsonType: "string",
              pattern: "^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$",
              description: "email должен быть валидной строкой email"
            },
            phone: {
              bsonType: "string",
              pattern: "^\\+?[0-9]{10,15}$",
              description: "phone должен содержать 10-15 цифр и может начинаться с +"
            }
          }
        }
      }
    }
  },
  validationLevel: "strict",
  validationAction: "error"
});

print("Результат установки валидации:");
printjson(validationResult);

print("\nПроверка вставки валидного документа:");
try {
  const validUser = {
    login: "temporary.valid.user",
    password: "valid_pass_123",
    first_name: "Тест",
    last_name: "Валидный",
    created_at: new Date(),
    status: "active",
    profile: {
      email: "valid.user@example.com",
      phone: "+79991234567"
    }
  };

  const insertValidResult = db.users.insertOne(validUser);
  print("Валидный документ успешно вставлен:");
  printjson(insertValidResult);

  const deleteValidResult = db.users.deleteOne({ _id: insertValidResult.insertedId });
  print("Тестовый валидный документ удалён:");
  printjson(deleteValidResult);
} catch (e) {
  print("Ошибка при вставке валидного документа:");
  print(e.message);
}

print("\nПроверка вставки невалидного документа:");
try {
  db.users.insertOne({
    login: "BAD LOGIN WITH SPACES",
    password: 12345,
    first_name: "",
    last_name: "Ошибка",
    created_at: "2026-04-21",
    status: "unknown",
    profile: {
      email: "not-an-email",
      phone: "abc"
    }
  });
  print("Ошибка: невалидный документ неожиданно был вставлен");
} catch (e) {
  print("Ожидаемо получена ошибка валидации:");
  print(e.message);
}

print("\nТекущее количество пользователей:");
print(db.users.countDocuments());
