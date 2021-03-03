import express from "express";
import mongodb, { ObjectID } from "mongodb";
import crypto from "crypto";
import DatasetGroup, { IGroupBackend } from "../models/DatasetGroup";
import { auth } from "./auth";
import { getUser } from "./user";
import { IUserBackend } from "../models/User";
import { ResBody } from "./index";
import { upload } from "./dataset";

const router = express.Router();

router.use(auth);

/**
 * get all groups
 */
router.get("/", async (req, res) => {
  const database: mongodb.Db = req.app.get("database");
  const collection = database.collection("groups");
  const caller = await getUser(database, (req as any).user.id);

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

  res.send({
    code: 200,
    data: {
      total,
      list: groups,
    },
  });
});

/**
 * add group
 */
router.post<{}, ResBody, { name: string; applications: string[] }>(
  "/",
  auth,
  async (req, res) => {
    const database: mongodb.Db = req.app.get("database");
    const caller = await getUser(database, (req as any).user.id);
    if (
      !caller.permission ||
      !caller.permission.groups ||
      !caller.permission.groups.create
    ) {
      res.send({
        code: 401,
        data: "No permission",
      });
      return;
    }

    const { name, applications = [] } = req.body;

    const collection = database.collection<IGroupBackend>("groups");

    const result = await collection.insertOne({
      name,
      creator: new ObjectID(caller.id),
      createTime: Date.now(),
      applications,
      datasets: [],
      users: [],
    });

    if (!result.insertedId) {
      res.send({
        code: 500,
        data: "Failed to add group",
      });
    }

    res.send({
      code: 200,
      data: result.insertedId,
    });
  }
);

/**
 * get group info
 */
router.get<{ id: string }, ResBody, {}>("/:id", async (req, res) => {
  const { id } = req.params;

  const db: mongodb.Db = req.app.get("database");
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
    res.send({
      code: 404,
      data: "group not found.",
    });
    return;
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

  res.send({
    code: 200,
    data: {
      ...group,
      creator: creator.name,
      datasets,
      users,
    },
  });
});

/**
 * delete a group
 */
router.delete<{ id: string }, ResBody, {}>("/:id", auth, async (req, res) => {
  const db: mongodb.Db = req.app.get("database");
  const caller = await getUser(db, (req as any).user.id);
  const { id } = req.params;
  if (
    (!caller.permission.group ||
      !caller.permission.group[id] ||
      !caller.permission.group[id].delete) &&
    (!caller.permission ||
      !caller.permission.groups ||
      !caller.permission.groups.delete)
  ) {
    res.send({
      code: 401,
      data: "No permission",
    });
    return;
  }

  const groupId = new ObjectID(id);
  const groups = db.collection("groups");
  const users = db.collection("users");
  const datasets = db.collection("datasets");
  const group = await groups.findOne<DatasetGroup>({ _id: groupId });
  let op = await groups.deleteOne({ _id: groupId });
  if (!op.result.ok) {
    res.send({
      code: 500,
      data: "Failed to delete group",
    });
    return;
  }

  op = await users.updateMany(
    { _id: { $in: group.users } },
    { $pull: { groups: groupId } }
  );
  if (!op.result.ok) {
    res.send({
      code: 500,
      data: "Unknown error",
    });
    return;
  }

  op = await datasets.updateMany(
    { _id: { $in: group.datasets } },
    { $pull: { groups: groupId } }
  );
  if (!op.result.ok) {
    res.send({
      code: 500,
      data: "Unknown error",
    });
    return;
  }

  res.send({
    code: 200,
  });
});

/**
 * update group base info
 */
router.put<{ id: string }, ResBody, { name: string; applications: string[] }>(
  "/:id",
  auth,
  async (req, res) => {
    const database: mongodb.Db = req.app.get("database");
    const caller = await getUser(database, (req as any).user.id);

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

    const collection = database.collection("groups");
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
  }
);

/**
 * add a dataset to a group
 */
router.post("/:id/datasets", upload.single("dataset"), async (req, res) => {
  // TODO: validate body
  const dataset = req.body;
  const { id } = req.params;
  const groupId = new ObjectID(id);

  const db: mongodb.Db = req.app.get("database");
  const datasets = db.collection("datasets");

  let datasetId = new ObjectID();
  if (dataset.id) {
    // add exist dataset
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
      res.send({
        code: 404,
        data: "Failed to add dataset",
      });
      return;
    }
  } else {
    const { name } = dataset;
    let path = "";
    if (req.file) {
      path = req.file.filename;
    } else {
      path = req.body.path;
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
    res.send({
      code: 500,
      data: "Failed to add dataset",
    });
    return;
  }

  res.send({
    code: 200,
  });
});

/**
 * remove a dataset from a group
 */
router.delete("/:gid/datasets/:did", async (req, res) => {
  // TODO: validate body
  const { gid, did } = req.params;
  const groupId = new ObjectID(gid);
  const datasetId = new ObjectID(did);

  const db: mongodb.Db = req.app.get("database");

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
    res.send({
      code: 500,
      data: "Failed to remove dataset",
    });
    return;
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
    res.send({
      code: 500,
      data: "Failed to remove dataset",
    });
    return;
  }

  res.send({
    code: 200,
  });
});

/**
 * add a user to a group
 */
router.post("/:id/users", async (req, res) => {
  // TODO: validate body
  const user = req.body;
  const { id } = req.params;
  const groupId = new ObjectID(id);

  const database: mongodb.Db = req.app.get("database");
  const users = database.collection("users");

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
      res.send({
        code: 404,
        data: "user not found.",
      });
      return;
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

  const groups = database.collection("groups");

  const result = await groups.updateOne(
    { _id: groupId },
    {
      $addToSet: {
        users: userId,
      },
    }
  );

  if (result.modifiedCount !== 1) {
    res.send({
      code: 500,
      data: "Failed to update",
    });
    return;
  }

  res.send({
    code: 200,
  });
});

/**
 * remove a user from a group
 */
router.delete("/:gid/users/:uid", async (req, res) => {
  // TODO: validate body
  const { gid, uid } = req.params;
  const groupId = new ObjectID(gid);
  const userId = new ObjectID(uid);

  const db: mongodb.Db = req.app.get("database");

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
    res.send({
      code: 500,
      data: "Failed to remove user",
    });
    return;
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
    res.send({
      code: 500,
      data: "Failed to remove user",
    });
    return;
  }

  res.send({
    code: 200,
  });
});

export default router;
