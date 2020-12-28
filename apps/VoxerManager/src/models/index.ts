import { options } from "../index";
import { MongoClient, Db } from "mongodb";

async function connect(): Promise<Db> {
  const client = new MongoClient(`mongodb://${options.database}?w=majority`, {
    useUnifiedTopology: true,
    useNewUrlParser: true,
  });

  try {
    // Connect the client to the server
    await client.connect();

    // Establish and verify connection
    const db = await client.db("voxer");
    await db.command({ ping: 1 });

    console.log("Connected successfully to server");

    return db;
  } catch (e) {
    console.log("Failed to connect to server: ", e.message);
    await client.close();
  }
}

export default connect;
