import express from 'express';
import DatasetRoutes from './dataset';
import PipelineRoutes from './pipeline';
import AnnotationRoutes from './annotation';

const router = express.Router();

router.use('/datasets', DatasetRoutes);
router.use('/pipelines', PipelineRoutes);
router.use('/annotations', AnnotationRoutes);

export default router;