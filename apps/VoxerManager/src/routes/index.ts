import express from "express";
import DatasetRoutes from "./dataset";
import PipelineRoutes from "./pipeline";
import AnnotationRoutes from "./annotation";
import GroupRoutes from "./group";
import RefineRoutes from "./refine";
import AutomRoutes from "./autom";

const router = express.Router();

router.use((req, res, next) => {
  res.status(200);
  next();
});
router.use("/datasets", DatasetRoutes);
router.use("/pipelines", PipelineRoutes);
router.use("/annotations", AnnotationRoutes);
router.use("/groups", GroupRoutes);
router.use("/refine", RefineRoutes);
router.use("/autom", AutomRoutes);

export default router;
