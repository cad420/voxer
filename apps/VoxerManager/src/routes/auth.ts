import express from "express";
import jwt from "express-jwt";

const router = express.Router();

/**
 * Login
 */
router.post("/login", async (req, res) => {
  const params = req.body;

  const id = params.id;

  res.send({
    code: 400,
    data: "Invalid login info",
  });
});

export { jwt };

export default router;
