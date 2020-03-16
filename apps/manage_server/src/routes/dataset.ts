import express from "express";
import multer from "multer";
import path from "path";
import store from "../models/Dataset";

const router = express.Router();

const target =
  process.env.UPLOAD_PATH || path.resolve(process.cwd(), "./public/");

const storage = multer.diskStorage({
  destination: (req, file, cb) => {
    cb(null, target);
  },
  filename: (req, file, cb) => {
    cb(null, file.originalname);
  }
});
const upload = multer({ storage });

router.post("/", upload.single("dataset"), (req, res) => {
  const name = req.file.filename;
  const dataset = {
    name,
    variables: [
      {
        name: "default",
        timesteps: 1,
        path: path.resolve(target, name)
      }
    ]
  };
  store.add(dataset);

  res.send({
    code: 200,
    data: name
  });
});

router.get("/", (req, res) => {
  const datasets = store.getAll();

  res.send({
    code: 200,
    data: Object.values(datasets)
  });
});

export default router;
