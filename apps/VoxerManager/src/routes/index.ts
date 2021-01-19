import express from "express";
import DatasetRoutes from "./dataset";
import PipelineRoutes from "./pipeline";
import AnnotationRoutes from "./annotation";
import GroupRoutes from "./group";
import UserRoutes from "./user";
import AuthRoutes from "./auth";

export type ResBody = {
  code: 200 | 400 | 401 | 404 | 403 | 500;
  data?: number | string | object | Array<any>;
};

const router = express.Router();

router.use((req, res, next) => {
  res.status(200);
  next();
});
router.use("/datasets", DatasetRoutes);
router.use("/pipelines", PipelineRoutes);
router.use("/annotations", AnnotationRoutes);
router.use("/groups", GroupRoutes);
router.use("/users", UserRoutes);
router.use("/auth", AuthRoutes);

export default router;
