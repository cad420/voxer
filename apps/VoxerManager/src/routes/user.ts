import crypto from "crypto";
import { FastifyInstance } from "fastify";
import mongodb, { ObjectID } from "mongodb";
import {
  IUser,
  IUserFrontEnd,
  IUserCreateInfo,
  IUserWithoutPwd,
} from "../models/User";

export function getUser(db: mongodb.Db, id: string): Promise<IUserWithoutPwd> {
  return db.collection("users").findOne<IUserWithoutPwd>(
    { _id: new ObjectID(id) },
    {
      projection: {
        _id: true,
        name: true,
        permission: true,
      },
    }
  );
}

async function routes(server: FastifyInstance) {
  const db = server.mongo.db;
  if (!db) {
    return;
  }

  const users = db.collection<IUser>("users");

  server.addHook("onRequest", async (request) => await request.jwtVerify());

  /**
   * get users
   */
  server.get<{
    Querystring: {
      page: string;
      size: string;
    };
  }>("/users", async (req) => {
    const caller = await getUser(db, req.user.id);
    if (
      !caller.permission ||
      !caller.permission.users ||
      !caller.permission.users.read
    ) {
      return {
        code: 401,
        data: "No permission",
      };
    }

    const users = db.collection("users");

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

    return {
      code: 200,
      data: {
        list,
        total,
      },
    };
  });

  /**
   * add user
   */
  server.post<{
    Body: IUserCreateInfo;
  }>("/users", async (req) => {
    const caller = await getUser(db, req.user.id);
    if (
      !caller.permission ||
      !caller.permission.users ||
      !caller.permission.users.create
    ) {
      return {
        code: 401,
        data: "No permission",
      };
    }

    const { name, password, permission } = req.body;

    // TODO: validate params

    const hashedPwd = crypto
      .createHash("sha256")
      .update(password)
      .digest("hex");

    const collection = db.collection("users");

    const row = await collection.insertOne({
      name,
      password: hashedPwd,
      permission,
      groups: [],
    });

    return {
      code: 200,
      data: row.insertedId.toString(),
    };
  });

  /**
   * delete user
   */
  server.delete<{
    Params: { id: string };
  }>("/users/:id", async (req) => {
    const caller = await getUser(db, req.user.id);
    if (
      !caller.permission ||
      !caller.permission.users ||
      !caller.permission.users.delete
    ) {
      return {
        code: 401,
        data: "No permission",
      };
    }

    const users = db.collection("users");

    const { id } = req.params;

    const result = await users.findOneAndDelete({
      _id: new ObjectID(id),
    });

    if (!result.ok) {
      return {
        code: 400,
      };
    }

    return { code: 200 };
  });

  /**
   * get user info
   */
  server.get<{
    Params: { id: string };
  }>("/users/:id", async (req, res) => {
    const { id } = req.params;
    const userId = new ObjectID(id);
    const caller = await getUser(db, req.user.id);
    if (
      caller._id !== userId &&
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

    const users = db.collection("users");

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
  server.put<{
    Params: { id: string };
    Body: {
      oldPassword?: string;
      password?: string;
    };
  }>("/users/:id", async (req) => {
    const { id } = req.params;
    const userId = new ObjectID(id);
    const caller = await getUser(db, req.user.id);
    const selfUpdate = userId === caller._id;
    if (
      !selfUpdate &&
      (!caller.permission ||
        !caller.permission.users ||
        !caller.permission.users.update)
    ) {
      return {
        code: 401,
        data: "No permission",
      };
      return;
    }

    const users = db.collection("users");

    const exist = await users.findOne<{
      password: string;
      name: string;
    }>(
      { _id: new ObjectID(id) },
      { projection: { password: true, name: true } }
    );

    if (!exist) {
      return {
        code: 400,
        data: "user does not exist",
      };
    }

    let password = exist.password;
    if (req.body.password) {
      const newPwd = req.body.password;
      if (selfUpdate) {
        const old = req.body.oldPassword;
        const hashed = crypto.createHash("sha256").update(old).digest("hex");
        if (hashed !== exist.password) {
          return {
            code: 400,
            data: "password not correct",
          };
        }
      }

      password = crypto.createHash("sha256").update(newPwd).digest("hex");
    }

    const result = await users.updateOne(
      { _id: new ObjectID(id) },
      { $set: { password } }
    );

    if (!result.result.ok) {
      return {
        code: 500,
        data: "Failed to update user",
      };
    }

    return {
      code: 200,
    };
  });

  /**
   * get user permissions
   */
  server.get<{
    Params: { id: string };
  }>("/users/:id/permission", async (req) => {
    const caller = await getUser(db, req.user.id);
    if (
      !caller.permission ||
      !caller.permission.users ||
      !caller.permission.users.read
    ) {
      return {
        code: 401,
        data: "No permission",
      };
    }

    const users = db.collection("users");

    const { id } = req.params;

    const exist = await users.findOne<{
      password: string;
      name: string;
      permission: IUserFrontEnd["permission"];
    }>(
      { _id: new ObjectID(id) },
      { projection: { password: true, name: true, permission: true } }
    );

    if (!exist) {
      return {
        code: 400,
        data: "user does not exist",
      };
    }

    return {
      code: 200,
      data: exist.permission,
    };
  });

  /**
   * update user permission
   */
  server.put<{
    Params: { id: string };
    Body: IUserFrontEnd["permission"];
  }>("/users/:id/permission", async (req, res) => {
    const caller = await getUser(db, req.user.id);
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

    const collection = db.collection("users");

    const { id } = req.params;

    const exist = await collection.findOne<{
      password: string;
      name: string;
    }>(
      { _id: new ObjectID(id) },
      { projection: { password: true, name: true } }
    );

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
}

export default routes;
