import crypto from "crypto";
import { FastifyInstance } from "fastify";
import { IUser } from "../models/User";

declare module "fastify-jwt" {
  interface FastifyJWT {
    payload: {
      id: string;
      name: string;
    };
  }
}

export function processPwd(password: string): string {
  return crypto.createHash("sha256").update(password).digest("hex");
}

async function routes(server: FastifyInstance) {
  const db = server.mongo.db;
  if (!db) {
    return;
  }

  const users = db.collection<IUser>("users");

  /**
   * create first user if not exist
   */
  server.get<{ Querystring: { name: string; password: string } }>(
    "/auth/initialize",
    async (req) => {
      const total = await users.countDocuments();

      if (total > 0) {
        return {
          code: 400,
          data: "Invalid request",
        };
      }

      const { name, password } = req.query;
      const result = await users.insertOne({
        name,
        password: processPwd(password),
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
      });
      if (!result || !result.insertedId) {
        return {
          code: 500,
          data: "Failed to initailize",
        };
      }

      return { code: 200, data: "Initialized." };
    }
  );

  /**
   * Login
   */
  server.post<{ Body: { name: string; password: string } }>(
    "/auth/login",
    async (req) => {
      const { name, password } = req.body;

      const users = db.collection("users");

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
        return {
          code: 400,
          data: "invalid login info",
        };
      }

      const hashedPwd = crypto
        .createHash("sha256")
        .update(password)
        .digest("hex");
      if (hashedPwd != exist.password) {
        return {
          code: 400,
          data: "invalid login info",
        };
      }

      const token = server.jwt.sign({
        id: exist.id,
        name,
      });
      return {
        code: 200,
        data: token,
      };
    }
  );
}

export default routes;
