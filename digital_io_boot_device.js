const Net = require("net");
const fs = require("fs");

const port = 2323;
const host = "localhost";
const client = new Net.Socket();

process.on("SIGINT", () => {
  console.log("SIGINT!");
  gracefulExit(0);
});

function gracefulExit(exitCode) {
  client.end(() => process.exit(exitCode));
  setTimeout(() => process.exit(exitCode), 3000);
}

const software = Uint8Array.from(
  fs.readFileSync("bin/software/0_instructions_actual.bin")
);

client.connect({ port: port, host: host }, () => {
  console.log("connected");
});

// The client can also receive data from the server by reading from its socket.
client.on("data", (chunk) => {
  const opcode = chunk.readUInt8();
  const opcodeHex = "0x" + opcode.toString(16);

  if (opcode == 0xaa) {
    console.log(opcodeHex, "Clearing counter");
    counter = 0;

    client.write(
      Uint8Array.from([software.length & 0xff, (software.length >> 8) & 0xff]),
      null,
      (err) => {
        if (err) console.error(err);
        else
          console.log(
            "Program size sent to client:",
            (software.length & 0xff).toString(16),
            ((software.length >> 8) & 0xff).toString(16)
          );
      }
    );

    client.write(software, null, (err) => {
      if (err) console.error(err);
      else console.log("Program sent to client");
    });
  } else {
    console.log(opcodeHex, "Unknown opcode");
  }
});

client.on("close", () => {
  gracefulExit(0);
});

client.on("error", (error) => {
  console.error("connection error", error);
  gracefulExit(1);
});

client.on("end", function () {
  console.log("connection end requested");
  gracefulExit(1);
});
