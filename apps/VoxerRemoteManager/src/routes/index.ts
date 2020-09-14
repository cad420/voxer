import express from "express";
import DatasetRoutes from "./dataset";
import PipelineRoutes from "./pipeline";
import AnnotationRoutes from "./annotation";
import GroupRoutes from "./group";

const router = express.Router();

router.use("/datasets", DatasetRoutes);
router.use("/pipelines", PipelineRoutes);
router.use("/annotations", AnnotationRoutes);
router.use("/groups", GroupRoutes);

export default router;
