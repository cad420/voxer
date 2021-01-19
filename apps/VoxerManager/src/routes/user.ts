import express from "express";
import mongodb, { ObjectID } from "mongodb";
import crypto from "crypto";
import { auth } from "./auth";
import User from "../models/User";
import { ResBody } from ".";

const router = express.Router();

/**
 * get users
 */
router.get<{}, ResBody, {}>("/", auth, async (req, res) => {
  const user = (req as any).user as User;
  if (
    !user.permission ||
    !user.permission.platform ||
    !user.permission.platform.readUsers
  ) {
    res.send({
      code: 401,
      data: "No permission",
    });
    return;
  }

  const database: mongodb.Db = req.app.get("database");
  const users = database.collection("users");

  const { page = 0, size = 10 } = req.query;
  const total = await users.countDocuments();
  const list = await users
    .find<{
      name: string;
    }>()
    .skip(page * size)
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
 * get user info
 */
router.get<{ id: string }, ResBody, {}>("/:id", auth, async (req, res) => {
  const { id } = req.params;
  const user = (req as any).user as User;
  if (user.id !== id) {
    res.send({
      code: 401,
      data: "No permission",
    });
    return;
  }

  const database: mongodb.Db = req.app.get("database");
  const users = database.collection("users");

  const result = await users.findOne<{ name: string }>(
    { _id: new ObjectID(id) },
    {
      projection: {
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
 * add user
 */
router.post<{}, ResBody, { name: string; password: string }>(
  "/",
  auth,
  async (req, res) => {
    const user = (req as any).user as User;
    if (
      !user.permission ||
      !user.permission.platform ||
      !user.permission.platform.createUsers
    ) {
      res.send({
        code: 401,
        data: "No priviledge",
      });
      return;
    }

    const { name, password } = req.body;

    // TODO: validate params

    const hashedPwd = crypto
      .createHash("sha256")
      .update(password)
      .digest("hex");

    const database: mongodb.Db = req.app.get("database");
    const collection = database.collection("users");

    const row = await collection.insertOne({
      name,
      password: hashedPwd,
    });

    res.send({
      code: 200,
      data: row.insertedId.toString(),
    });
  }
);

/**
 * delete user
 */
router.delete<{}, ResBody, { id: string }>("/", auth, async (req, res) => {
  const user = (req as any).user as User;
  if (
    !user.permission ||
    !user.permission.platform ||
    !user.permission.platform.deleteUsers
  ) {
    res.send({
      code: 401,
      data: "No priviledge",
    });
    return;
  }

  const database: mongodb.Db = req.app.get("database");
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
 * User update info
 */
router.put<
  {
    id: string;
  },
  ResBody,
  {
    name?: string;
    oldPassword?: string;
    password?: string;
  }
>("/:id", async (req, res) => {
  const user = (req as any).user as User;
  if (
    !user.permission ||
    !user.permission.platform ||
    !user.permission.platform.updateUsers
  ) {
    res.send({
      code: 401,
      data: "No priviledge",
    });
    return;
  }

  const database: mongodb.Db = req.app.get("database");
  const users = database.collection("users");

  const { id } = req.params;

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
    const old = req.body.oldPassword;
    const hashed = crypto.createHash("sha256").update(old).digest("hex");
    if (hashed !== exist.password) {
      res.send({
        code: 400,
        data: "password not correct",
      });
      return;
    }

    password = req.body.password;
  }

  const name = req.body.name || exist.name;

  const result = await users.updateOne(
    { _id: new ObjectID(id) },
    { $set: { name, password } }
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
