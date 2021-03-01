import express from "express";
import expressJWT from "express-jwt";
import mongodb from "mongodb";
import jwt from "jsonwebtoken";
import crypto from "crypto";
import { ResBody } from ".";
import { IUserBackend } from "../models/User";

const secret = "shhhhhhared-secret";
const auth = expressJWT({
  secret,
  algorithms: ["HS256"],
});

const router = express.Router();

/**
 * create first user if not exist
 */
router.get<{ name: string; password: string }, ResBody | string, {}>(
  "/initialize",
  async (req, res) => {
    const database: mongodb.Db = req.app.get("database");
    const users = database.collection("users");
    const total = await users.countDocuments();

    if (total > 0) {
      res.send({
        code: 400,
        data: "Invalid request",
      });
      return;
    }

    const { name, password } = req.query;
    const admin: IUserBackend = {
      name,
      password: crypto.createHash("sha256").update(password).digest("hex"),
      permission: {
        users: {
          create: true,
          delete: true,
          read: true,
          update: true,
        },
        groups: {
          create: true,
          delete: true,
          read: true,
          update: true,
        },
        group: {},
      },
    };

    const result = await users.insertOne(admin);
    if (result && result.insertedId) {
      res.send("Initialized.");
    } else {
      res.send({
        code: 500,
        data: "Failed to initailize",
      });
    }
  }
);

/**
 * Login
 */
router.post<{}, ResBody, { name: string; password: string }>(
  "/login",
  async (req, res) => {
    const { name, password } = req.body;

    const database: mongodb.Db = req.app.get("database");
    const users = database.collection("users");

    const exist = await users.findOne<{ id: string; password: string }>(
      { name },
      {
        projection: {
          _id: false,
          id: {
            $toString: "$_id",
          },
          password: true,
        },
      }
    );

    if (!exist) {
      res.send({
        code: 400,
        data: "invalid login info",
      });
      return;
    }

    const hashedPwd = crypto
      .createHash("sha256")
      .update(password)
      .digest("hex");
    if (hashedPwd != exist.password) {
      res.send({
        code: 400,
        data: "invalid login info",
      });
      return;
    }

    const token = jwt.sign(
      {
        id: exist.id,
        name,
      },
      secret
    );
    res.send({
      code: 200,
      data: token,
    });
  }
);

export { auth };

export default router;
