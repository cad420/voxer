import fs from "fs";
import crypto from "crypto";
import { resolve } from "path";
import { ObjectID } from "mongodb";
import { FastifyInstance } from "fastify";
import { pump } from "./dataset";
import { getUser } from "./user";
import { options } from "../index";
import { IUserBackend } from "../models/User";
import DatasetGroup, { IGroupBackend } from "../models/DatasetGroup";

async function routes(server: FastifyInstance) {
  const db = server.mongo.db;
  if (!db) {
    return;
  }

  server.addHook("onRequest", async (request) => await request.jwtVerify());

  /**
   * get all groups
   */
  server.get<{
    Querystring: {
      page: string;
      size: string;
    };
  }>("/groups", async (req) => {
    const collection = db.collection("groups");
    const caller = await getUser(db, req.user.id);

    const ids = Object.keys(caller.permission.group).map(
      (id) => new ObjectID(id)
    );
    const query: any = { _id: { $in: ids } };

    let page = parseInt(req.query.page);
    if (isNaN(page) || page <= 0) {
      page = 1;
    }
    let size = parseInt(req.query.size);
    if (isNaN(page) || size <= 0) {
      size = 10;
    }

    if (
      caller.permission &&
      caller.permission.groups &&
      caller.permission.groups.read
    ) {
      delete query._id;
    }

    const total = await collection.countDocuments(query);
    const groups = await collection
      .find<{ id: string; name: string }>(query, {
        projection: {
          _id: false,
          id: {
            $toString: "$_id",
          },
          name: true,
          createTime: true,
          creator: true,
          applications: true,
        },
      })
      .skip((page - 1) * size)
      .limit(size)
      .toArray();

    return {
      code: 200,
      data: {
        total,
        list: groups,
      },
    };
  });

  /**
   * add group
   */
  server.post<{
    Body: { name: string; applications: string[] };
  }>("/groups", async (req) => {
    const caller = await getUser(db, req.user.id);
    if (
      !caller.permission ||
      !caller.permission.groups ||
      !caller.permission.groups.create
    ) {
      return {
        code: 401,
        data: "No permission",
      };
    }

    const { name, applications = [] } = req.body;

    const collection = db.collection<IGroupBackend>("groups");

    const result = await collection.insertOne({
      name,
      creator: caller._id,
      createTime: Date.now(),
      applications,
      datasets: [],
      users: [],
    });

    if (!result.insertedId) {
      return {
        code: 500,
        data: "Failed to add group",
      };
    }

    return {
      code: 200,
      data: result.insertedId,
    };
  });

  /**
   * get group info
   */
  server.get<{ Params: { id: string } }>("/groups/:id", async (req) => {
    const { id } = req.params;

    const collection = db.collection("groups");
    const group = await collection.findOne<DatasetGroup>(
      {
        _id: new ObjectID(id),
      },
      {
        projection: {
          _id: false,
          id: {
            $toString: "$_id",
          },
          name: true,
          createTime: true,
          creator: true,
          applications: true,
          datasets: true,
          users: true,
        },
      }
    );

    if (!group) {
      return {
        code: 404,
        data: "group not found.",
      };
    }

    const datasets = await db
      .collection("datasets")
      .find(
        { _id: { $in: group.datasets } },
        {
          projection: {
            _id: false,
            id: {
              $toString: "$_id",
            },
            name: true,
            createTime: true,
            creator: true,
          },
        }
      )
      .toArray();

    const users = await db
      .collection("users")
      .find(
        { _id: { $in: group.users } },
        { projection: { _id: false, id: { $toString: "$_id" }, name: true } }
      )
      .toArray();

    const creator = await db
      .collection("users")
      .findOne<IUserBackend>(
        { _id: group.creator },
        { projection: { _id: false, name: true } }
      );

    return {
      code: 200,
      data: {
        ...group,
        creator: creator.name,
        datasets,
        users,
      },
    };
  });

  /**
   * delete a group
   */
  server.delete<{ Params: { id: string } }>("/groups/:id", async (req) => {
    const caller = await getUser(db, req.user.id);
    const { id } = req.params;
    if (
      (!caller.permission.group ||
        !caller.permission.group[id] ||
        !caller.permission.group[id].delete) &&
      (!caller.permission ||
        !caller.permission.groups ||
        !caller.permission.groups.delete)
    ) {
      return {
        code: 401,
        data: "No permission",
      };
    }

    const groupId = new ObjectID(id);
    const groups = db.collection("groups");
    const users = db.collection("users");
    const datasets = db.collection("datasets");
    const group = await groups.findOne<DatasetGroup>({ _id: groupId });
    let op = await groups.deleteOne({ _id: groupId });
    if (!op.result.ok) {
      return {
        code: 500,
        data: "Failed to delete group",
      };
    }

    op = await users.updateMany(
      { _id: { $in: group.users } },
      { $pull: { groups: groupId } }
    );
    if (!op.result.ok) {
      return {
        code: 500,
        data: "Unknown error",
      };
    }

    op = await datasets.updateMany(
      { _id: { $in: group.datasets } },
      { $pull: { groups: groupId } }
    );
    if (!op.result.ok) {
      return {
        code: 500,
        data: "Unknown error",
      };
    }

    return {
      code: 200,
    };
  });

  /**
   * update group base info
   */
  server.put<{
    Params: { id: string };
    Body: { name: string; applications: string[] };
  }>("/groups/:id", async (req, res) => {
    const caller = await getUser(db, req.user.id);
    if (
      !caller.permission ||
      !caller.permission.groups ||
      !caller.permission.groups.update
    ) {
      res.send({
        code: 401,
        data: "No permission",
      });
      return;
    }

    const { id } = req.params;
    const groupId = new ObjectID(id);

    const collection = db.collection("groups");
    const exist = await collection.findOne<DatasetGroup>({ _id: groupId });
    if (!exist) {
      res.send({
        code: 404,
        data: "group not found",
      });
      return;
    }

    const { name = exist.name, applications = exist.applications } = req.body;
    await collection.updateOne(
      { _id: groupId },
      {
        $set: {
          name,
          applications,
        },
      }
    );

    res.send({
      code: 200,
    });
  });

  /**
   * add a dataset to a group
   */
  server.post<{
    Params: { id: string };
    Body:
      | {
          id: string;
          name: string;
          dataset: Buffer;
          path?: string;
        }
      | undefined;
  }>("/groups/:id/datasets", async (req) => {
    // TODO: validate body
    const { id } = req.params;
    const groupId = new ObjectID(id);

    const datasets = db.collection("datasets");

    let name = "";
    let datasetId = new ObjectID();
    if (req.body && req.body.id) {
      // add exist dataset
      const dataset = req.body;
      name = dataset.name;
      datasetId = new ObjectID(dataset.id);
      const result = await datasets.updateOne(
        { _id: datasetId },
        {
          $addToSet: {
            groups: groupId,
          },
        }
      );
      if (!result.modifiedCount) {
        return {
          code: 404,
          data: "Failed to add dataset",
        };
      }
    } else {
      let path = "";
      if (req.body) {
        path = req.body.path;
        name = req.body.name;
      } else {
        const data = await req.file();
        const filepath = resolve(options.storage, path);
        await pump(data.file, fs.createWriteStream(filepath));
        path = data.filename;
        name = (data.fields.name as any).value;
      }

      const exist = await datasets.findOne(
        { path },
        { projection: { _id: true } }
      );

      if (exist) {
        datasetId = exist._id as ObjectID;
        await datasets.updateOne(
          {
            _id: datasetId,
          },
          { $push: { groups: groupId } }
        );
      } else {
        const result = await datasets.insertOne({
          path, // TODO
          name,
          dimensions: [1, 1, 1],
          histogram: [],
          range: [1, 1],
          groups: [groupId],
        });
        datasetId = result.insertedId;
      }
    }

    const groups = db.collection("groups");
    const result = await groups.updateOne(
      { _id: groupId },
      {
        $addToSet: {
          datasets: datasetId,
        },
      }
    );

    if (result.modifiedCount !== 1) {
      return {
        code: 500,
        data: "Failed to add dataset",
      };
    }

    return {
      code: 200,
    };
  });

  /**
   * remove a dataset from a group
   */
  server.delete<{
    Params: {
      gid: string;
      did: string;
    };
  }>("/groups/:gid/datasets/:did", async (req) => {
    // TODO: validate body
    const { gid, did } = req.params;
    const groupId = new ObjectID(gid);
    const datasetId = new ObjectID(did);

    const groups = db.collection("groups");
    const datasets = db.collection("datasets");

    let result = await groups.updateOne(
      { _id: groupId },
      {
        $pull: {
          datasets: datasetId,
        },
      }
    );

    if (result.modifiedCount !== 1) {
      return {
        code: 500,
        data: "Failed to remove dataset",
      };
    }

    result = await datasets.updateOne(
      { _id: datasetId },
      {
        $pull: {
          groups: groupId,
        },
      }
    );

    if (result.modifiedCount !== 1) {
      return {
        code: 500,
        data: "Failed to remove dataset",
      };
    }

    return {
      code: 200,
    };
  });

  /**
   * add a user to a group
   */
  server.post<{
    Params: { id: string };
    Body: {
      id?: string;
      name: String;
      password: string;
    };
  }>("/groups/:id/users", async (req) => {
    // TODO: validate body
    const user = req.body;
    const { id } = req.params;
    const groupId = new ObjectID(id);

    const users = db.collection("users");

    let userId = new ObjectID();
    if (user.id) {
      // add exist user
      userId = new ObjectID(user.id);
      const result = await users.updateOne(
        { _id: userId },
        {
          $addToSet: {
            groups: groupId,
          },
        }
      );
      if (!result.modifiedCount) {
        return {
          code: 404,
          data: "user not found.",
        };
      }
    } else {
      // new user
      const hashedPwd = crypto
        .createHash("sha256")
        .update(user.password)
        .digest("hex");
      const newUser = {
        name: user.name,
        password: hashedPwd,
        groups: [groupId],
        permissions: {},
      };
      const result = await users.insertOne(newUser);
      userId = result.insertedId;
    }

    const groups = db.collection("groups");

    const result = await groups.updateOne(
      { _id: groupId },
      {
        $addToSet: {
          users: userId,
        },
      }
    );

    if (result.modifiedCount !== 1) {
      return {
        code: 500,
        data: "Failed to update",
      };
    }

    return {
      code: 200,
    };
  });

  /**
   * remove a user from a group
   */
  server.delete<{
    Params: {
      gid: string;
      uid: string;
    };
  }>("/groups/:gid/users/:uid", async (req) => {
    // TODO: validate body
    const { gid, uid } = req.params;
    const groupId = new ObjectID(gid);
    const userId = new ObjectID(uid);

    const groups = db.collection("groups");
    const users = db.collection("users");

    let result = await groups.updateOne(
      { _id: groupId },
      {
        $pull: {
          users: userId,
        },
      }
    );

    if (result.modifiedCount !== 1) {
      return {
        code: 500,
        data: "Failed to remove user",
      };
    }

    result = await users.updateOne(
      { _id: userId },
      {
        $pull: {
          groups: groupId,
        },
      }
    );

    if (result.modifiedCount !== 1) {
      return {
        code: 500,
        data: "Failed to remove user",
      };
    }

    return {
      code: 200,
    };
  });
}

export default routes;
