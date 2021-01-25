import express from "express";
import mongodb, { ObjectID } from "mongodb";
import crypto from "crypto";
import { auth } from "./auth";
import User from "../models/User";
import { ResBody } from ".";

export function getUser(db: mongodb.Db, id: string): Promise<User> {
  return db.collection("users").findOne<User>(
    { _id: new ObjectID(id) },
    {
      projection: {
        _id: false,
        id: { $toString: "$_id" },
        name: true,
        permission: true,
      },
    }
  );
}

const router = express.Router();

router.use(auth);

/**
 * get users
 */
router.get<{}, ResBody>("/", async (req, res) => {
  const database: mongodb.Db = req.app.get("database");
  const caller = await getUser(database, (req as any).user.id);
  if (
    !caller.permission ||
    !caller.permission.users ||
    !caller.permission.users.read
  ) {
    res.send({
      code: 401,
      data: "No permission",
    });
    return;
  }

  const users = database.collection("users");

  let page = parseInt(req.query.page);
  if (isNaN(page) || page <= 0) {
    page = 1;
  }
  let size = parseInt(req.query.size);
  if (isNaN(page) || size <= 0) {
    size = 10;
  }
  const total = await users.countDocuments();
  const list = await users
    .find<{
      name: string;
    }>(
      {},
      {
        projection: {
          _id: false,
          id: { $toString: "$_id" },
          name: true,
          permission: true,
        },
      }
    )
    .skip((page - 1) * size)
    .limit(size)
    .toArray();

  res.send({
    code: 200,
    data: {
      list,
      total,
    },
  });
});

/**
 * add user
 */
router.post<{}, ResBody, User>("/", async (req, res) => {
  const database: mongodb.Db = req.app.get("database");
  const caller = await getUser(database, (req as any).user.id);
  if (
    !caller.permission ||
    !caller.permission.users ||
    !caller.permission.users.create
  ) {
    res.send({
      code: 401,
      data: "No permission",
    });
    return;
  }

  const { name, password, permission } = req.body;

  // TODO: validate params

  const hashedPwd = crypto.createHash("sha256").update(password).digest("hex");

  const collection = database.collection("users");

  const row = await collection.insertOne({
    name,
    password: hashedPwd,
    permission,
  });

  res.send({
    code: 200,
    data: row.insertedId.toString(),
  });
});

/**
 * delete user
 */
router.delete<{}, ResBody, { id: string }>("/", async (req, res) => {
  const database: mongodb.Db = req.app.get("database");
  const caller = await getUser(database, (req as any).user.id);
  if (
    !caller.permission ||
    !caller.permission.users ||
    !caller.permission.users.delete
  ) {
    res.send({
      code: 401,
      data: "No permission",
    });
    return;
  }

  const users = database.collection("users");

  const { id } = req.body;

  const result = await users.findOneAndDelete({
    _id: new ObjectID(id),
  });

  if (!result.ok) {
    res.send({
      code: 400,
    });
    return;
  }

  res.send({ code: 200 });
});

/**
 * get user info
 */
router.get<{ id: string }, ResBody>("/:id", async (req, res) => {
  const { id } = req.params;
  const database: mongodb.Db = req.app.get("database");
  const caller = await getUser(database, (req as any).user.id);
  if (
    caller.id !== id &&
    (!caller.permission ||
      !caller.permission.users ||
      !caller.permission.users.read)
  ) {
    res.send({
      code: 401,
      data: "No permission",
    });
    return;
  }

  const users = database.collection("users");

  const result = await users.findOne<{ name: string }>(
    { _id: new ObjectID(id) },
    {
      projection: {
        _id: false,
        id: { $toString: "$_id" },
        name: true,
      },
    }
  );

  res.send({
    code: 200,
    data: result,
  });
});

/**
 * update user info
 */
router.put<
  {
    id: string;
  },
  ResBody,
  {
    oldPassword?: string;
    password?: string;
  }
>("/:id", async (req, res) => {
  const { id } = req.params;
  const database: mongodb.Db = req.app.get("database");
  const caller = await getUser(database, (req as any).user.id);
  const selfUpdate = id === caller.id;
  if (
    !selfUpdate &&
    (!caller.permission ||
      !caller.permission.users ||
      !caller.permission.users.update)
  ) {
    res.send({
      code: 401,
      data: "No permission",
    });
    return;
  }

  const users = database.collection("users");

  const exist = await users.findOne<{
    password: string;
    name: string;
  }>({ _id: new ObjectID(id) }, { projection: { password: true, name: true } });

  if (!exist) {
    res.send({
      code: 400,
      data: "user does not exist",
    });
    return;
  }

  let password = exist.password;
  if (req.body.password) {
    const newPwd = req.body.password;
    if (selfUpdate) {
      const old = req.body.oldPassword;
      const hashed = crypto.createHash("sha256").update(old).digest("hex");
      if (hashed !== exist.password) {
        res.send({
          code: 400,
          data: "password not correct",
        });
        return;
      }
    }

    password = crypto.createHash("sha256").update(newPwd).digest("hex");
  }

  const result = await users.updateOne(
    { _id: new ObjectID(id) },
    { $set: { password } }
  );

  if (!result.result.ok) {
    res.send({
      code: 500,
      data: "Failed to update user",
    });
    return;
  }

  res.send({
    code: 200,
  });
});

/**
 * delete user
 */
router.delete<
  {
    id: string;
  },
  ResBody
>("/:id", async (req, res) => {
  const { id } = req.params;
  const database: mongodb.Db = req.app.get("database");
  const caller = await getUser(database, (req as any).user.id);
  if (caller.id === id) {
    res.send({
      code: 401,
      data: "Cannot delete self.",
    });
    return;
  }

  if (
    !caller.permission ||
    !caller.permission.users ||
    !caller.permission.users.delete
  ) {
    res.send({
      code: 401,
      data: "No permission",
    });
    return;
  }

  const collection = database.collection("users");

  const exist = await collection.findOne<{
    password: string;
    name: string;
  }>({ _id: new ObjectID(id) }, { projection: { password: true, name: true } });

  if (!exist) {
    res.send({
      code: 400,
      data: "user does not exist",
    });
    return;
  }

  const result = await collection.deleteOne({ _id: new ObjectID(id) });

  if (result.deletedCount !== 1) {
    res.send({
      code: 500,
      data: "Failed to update user",
    });
    return;
  }

  res.send({
    code: 200,
  });
});

/**
 * get user permissions
 */
router.get<
  {
    id: string;
  },
  ResBody,
  {}
>("/:id/permission", async (req, res) => {
  const database: mongodb.Db = req.app.get("database");
  const caller = await getUser(database, (req as any).user.id);
  if (
    !caller.permission ||
    !caller.permission.users ||
    !caller.permission.users.read
  ) {
    res.send({
      code: 401,
      data: "No permission",
    });
    return;
  }

  const users = database.collection("users");

  const { id } = req.params;

  const exist = await users.findOne<{
    password: string;
    name: string;
    permission: User["permission"];
  }>(
    { _id: new ObjectID(id) },
    { projection: { password: true, name: true, permission: true } }
  );

  if (!exist) {
    res.send({
      code: 400,
      data: "user does not exist",
    });
    return;
  }

  res.send({
    code: 200,
    data: exist.permission,
  });
});

/**
 * update user permission
 */
router.put<
  {
    id: string;
  },
  ResBody,
  User["permission"]
>("/:id/permission", async (req, res) => {
  const database: mongodb.Db = req.app.get("database");
  const caller = await getUser(database, (req as any).user.id);
  if (
    !caller.permission ||
    !caller.permission.users ||
    !caller.permission.users.update
  ) {
    res.send({
      code: 401,
      data: "No permission",
    });
    return;
  }

  const collection = database.collection("users");

  const { id } = req.params;

  const exist = await collection.findOne<{
    password: string;
    name: string;
  }>({ _id: new ObjectID(id) }, { projection: { password: true, name: true } });

  if (!exist) {
    res.send({
      code: 400,
      data: "user does not exist",
    });
    return;
  }

  const { users = {}, groups = {} } = req.body;
  const result = await collection.updateOne(
    { _id: new ObjectID(id) },
    { $set: { permission: { users, groups } } }
  );

  if (result.modifiedCount !== 1) {
    res.send({
      code: 500,
      data: "Failed to update user",
    });
    return;
  }

  res.send({
    code: 200,
  });
});

export default router;
