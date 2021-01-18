import express from "express";
import mongodb, { ObjectID } from "mongodb";
import crypto from "crypto";

const router = express.Router();

/**
 * Admin add user
 */
router.post("/", async (req, res) => {
  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("users");

  const params = req.body;

  // TODO: validate params

  const password: string = params.password;
  const hashedPwd = crypto.createHash("md5").update(password).digest("hex");

  const row = await collection.insertOne({
    name: params.name,
    password: hashedPwd,
  });

  res.send({
    code: 200,
    data: row.insertedId.toString(),
  });
});

/**
 * Admin delete user
 */
router.delete("/", async (req, res) => {
  const database: mongodb.Db = req.app.get("database");
  const users = database.collection("users");

  const params = req.body;

  const id = params.id;

  const result = await users.findOneAndDelete({
    _id: new ObjectID(id),
  });

  if (result.ok) {
    res.send({
      code: 200,
    });
  } else {
    res.send({
      code: 400,
    });
  }
});

/**
 * User update info
 */
router.put("/:id", async (req, res) => {
  const database: mongodb.Db = req.app.get("database");
  const users = database.collection("users");

  const { id } = req.params;

  const exist = await users.findOne({ _id: new ObjectID(id) });

  if (!exist) {
    res.send({
      code: 400,
      data: "user does not exist",
    });
    return;
  }
});

export default router;
