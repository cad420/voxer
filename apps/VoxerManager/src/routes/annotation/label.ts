import { FastifyInstance } from "fastify";

async function routes(server: FastifyInstance) {
  /**
   * get group's labels
   */
  server.get("/groups/:id/labels/", async (req, res) => {});

  /**
   * add a label
   */
  server.post("/groups/:id/labels", async (req, res) => {});

  /**
   * update a label
   */
  server.put("/groups/:gid/labels/:lid", async (req, res) => {});

  /**
   * delete a label
   */
  server.delete("/groups/:gid/labels/:lid", async (req, res) => {});
}

export default routes;
