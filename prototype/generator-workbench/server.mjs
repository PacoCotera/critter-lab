import { createServer } from "node:http";
import { readFile } from "node:fs/promises";
import { fileURLToPath } from "node:url";
import { resolve } from "node:path";
import { catalogue, evaluate } from "./evaluate.mjs";

const files = new Map([
  ["/", ["index.html", "text/html; charset=utf-8"]],
  ["/app.mjs", ["app.mjs", "text/javascript; charset=utf-8"]],
  ["/style.css", ["style.css", "text/css; charset=utf-8"]],
]);
const maximumBodyBytes = 65536;

export function makeServer() {
  return createServer(async (request, response) => {
    const send = (status, type, content) => {
      response.writeHead(status, { "Content-Type": type, "Cache-Control": "no-store", "X-Content-Type-Options": "nosniff" });
      response.end(content);
    };
    const json = (status, body) => send(status, "application/json; charset=utf-8", JSON.stringify(body));
    // This is a local developer tool; reject remote browser origins instead of exposing a service.
    const host = request.headers.host;
    if (!host || !/^127\.0\.0\.1:\d+$/.test(host)) return json(403, { error: "Loopback Host required" });
    if (request.headers.origin && request.headers.origin !== `http://${host}`) return json(403, { error: "Same-origin request required" });
    try {
      if (request.method === "GET" && request.url === "/api/catalogue") return json(200, catalogue());
      if (request.method === "POST" && request.url === "/api/evaluate") {
        const chunks = [];
        let size = 0;
        for await (const chunk of request) {
          size += chunk.length;
          if (size > maximumBodyBytes) return json(413, { error: "Experiment exceeds 64 KiB" });
          chunks.push(chunk);
        }
        let input;
        try { input = JSON.parse(Buffer.concat(chunks).toString("utf8")); }
        catch { return json(400, { error: "Invalid JSON" }); }
        const result = evaluate(input);
        return json(result.status === "resolved" ? 200 : 422, result);
      }
      const file = files.get(request.url);
      if (request.method === "GET" && file) {
        return send(200, file[1], await readFile(new URL(file[0], import.meta.url)));
      }
      return json(404, { error: "Not found" });
    } catch {
      return json(500, { error: "Evaluation failed; no result produced" });
    }
  });
}

if (process.argv[1] && resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
  const server = makeServer();
  server.listen(4381, "127.0.0.1", () => console.log("Generator workbench: http://127.0.0.1:4381"));
}
