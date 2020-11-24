import run from "./server";

let port = 3001;
if (process.env.PORT) {
  const tmp = parseInt(process.env.PORT);
  if (!isNaN(tmp) && tmp > 0 && tmp < 65535) {
    port = tmp;
  }
}

run(port);
