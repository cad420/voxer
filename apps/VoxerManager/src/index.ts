import { Command } from "commander";
import { version } from "../package.json";
import Server from "./Server";

const program = new Command();

program
  .version(version)
  .option("--port <port>", "port listening")
  .option("--database <file>")
  .option("--storage <directory>", "directory to store data")
  .option("--serve <directory>", "serve static files in directory")
  .parse(process.argv);

export const options = {
  port: 3001,
  database: "127.0.0.1:27017",
  storage: "public",
  serve: "public",
};

if (program.port) {
  const port = parseInt(program.port);
  if (!isNaN(port) && port > 0 && port < 65536) {
    options.port = port;
  }
}

if (program.database) {
  options.database = program.database;
}

if (program.storage) {
  options.storage = program.storage;
}

if (program.serve) {
  options.serve = program.serve;
}

const server = new Server(options);
server.listen();
