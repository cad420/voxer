import { FastifyDynamicSwaggerOptions } from "fastify-swagger";

const config: FastifyDynamicSwaggerOptions["openapi"] = {
  info: {
    title: "VoxerManager API",
    description: "API documentation for VoxerManager",
    version: "0.1.0",
  },
  components: {
    securitySchemes: {
      bearerAuth: {
        type: "http",
        scheme: "bearer",
        bearerFormat: "JWT",
      },
    },
  },
  security: [{ bearerAuth: [] }],
};

export default config;
