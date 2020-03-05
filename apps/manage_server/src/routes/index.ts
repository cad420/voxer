import express from 'express';
import DatasetRoutes from './dataset';
import PipelineRoutes from './pipeline';

const router = express.Router();
router.use('/datasets', DatasetRoutes);
router.use('/pipelines', PipelineRoutes);

export default router;