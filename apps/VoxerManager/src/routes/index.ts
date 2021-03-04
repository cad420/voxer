import { FastifyInstance } from "fastify";
import AuthRoutes from "./auth";
import UserRoutes from "./user";
import GroupRoutes from "./group";
import DatasetRoutes from "./dataset";
import PipelineRoutes from "./pipeline/pipeline";
import AnnotationRoutes from "./annotation/annotation";

export type ResBody = {
  code: 200 | 400 | 401 | 404 | 403 | 500;
  data?: number | string | object | Array<any>;
};

export interface IReply {
  code: 200 | 400 | 401 | 404 | 403 | 500;
  data?: number | string | object | Array<any>;
}

export default async (server: FastifyInstance) => {
  server.register(AuthRoutes);
  server.register(DatasetRoutes);
  server.register(UserRoutes);
  server.register(GroupRoutes);

  // annotation application's routes
  server.register(AnnotationRoutes);

  // pipeline application's routes
  server.register(PipelineRoutes);
};
