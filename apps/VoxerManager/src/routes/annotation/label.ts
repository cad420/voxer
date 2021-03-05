import { FastifyInstance } from "fastify";
import { ObjectID } from "mongodb";
import Label from "../../models/Labels";

async function routes(server: FastifyInstance) {
  const db = server.mongo.db;
  if (!db) {
    return;
  }

  const groups = db.collection("groups");

  /**
   * add a label for a group
   */
  server.post<{
    Params: {
      gid: string;
    };
    Body: Label;
  }>("/groups/:gid/labels", async (req, res) => {
    // TODO: validate body
    const label = req.body;
    const { gid } = req.params;

    const collection = db.collection("groups");

    const labelId = new ObjectID();
    const result = await collection.updateOne(
      { _id: new ObjectID(gid) },
      {
        $push: {
          labels: {
            id: labelId,
            name: label.name,
            color: label.color,
            type: label.type,
          },
        },
      }
    );

    if (result.modifiedCount !== 1) {
      res.send({
        code: 400,
        data: "Failed to update",
      });
      return;
    }

    res.send({
      code: 200,
      data: labelId.toHexString(),
    });
  });

  /**
   * remove a label for a group
   */
  server.delete<{
    Params: {
      gid: string;
      lid: string;
    };
  }>("/groups/:gid/labels/:lid", async (req, res) => {
    // TODO: validate body
    const { gid, lid } = req.params;
    const groupId = new ObjectID(gid);
    const labelId = new ObjectID(lid);

    const group = await groups.findOne(
      {
        _id: groupId,
      },
      {
        projection: {
          labels: true,
          datasets: true,
        },
      }
    );

    if (!group) {
      res.send({
        code: 404,
        data: "group not found",
      });
      return;
    }

    // TODO: too much overhead
    Object.values(group.datasets).forEach((item) => {
      if (item.labels[lid]) {
        delete item.labels[lid];
      }
    });

    const result = await groups.updateOne(
      { _id: new ObjectID(groupId) },
      {
        $pull: {
          labels: {
            id: new ObjectID(labelId),
          },
        },
        $set: {
          datasets: group.datasets,
        },
      }
    );

    if (result.modifiedCount !== 1) {
      res.send({
        code: 400,
        data: "Failed to update",
      });
      return;
    }

    res.send({
      code: 200,
    });
  });
  /**
   * get group's labels
   */
  server.get("/groups/:id/labels/", async (req, res) => {});

  /**
   * add a label
   */
  server.post("/groups/:id/labels", async (req, res) => {});

  /**
   * update a label
   */
  server.put("/groups/:gid/labels/:lid", async (req, res) => {});

  /**
   * delete a label
   */
  server.delete("/groups/:gid/labels/:lid", async (req, res) => {});
}

export default routes;
