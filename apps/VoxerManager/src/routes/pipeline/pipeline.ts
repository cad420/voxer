import { FastifyInstance } from "fastify";

async function routes(server: FastifyInstance) {
  /**
   * get group's pipelines
   */
  server.get("/groups/:id/pipeline/", async (req, res) => {});

  /**
   * add a pipeline
   */
  server.post("/groups/:id/pipeline", async (req, res) => {});

  /**
   * update a pipeline
   */
  server.put("/groups/:gid/pipeline/:pid", async (req, res) => {});

  /**
   * delete a pipeline
   */
  server.delete("/groups/:gid/pipeline/:pid", async (req, res) => {});
}

export default routes;
