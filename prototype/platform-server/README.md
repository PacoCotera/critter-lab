# Website, genome workbench and simulator gateway

One public origin presents three separate applications:

- `/` serves only the committed `website/dist/` static product introduction.
- `/genome/` forwards to the existing Node workbench at `127.0.0.1:4381`.
- `/sandbox/` forwards to the existing native presenter at `127.0.0.1:4190`.

`/genome` and `/sandbox` redirect to their canonical trailing-slash paths. Root
`/api/*` continues to reach the native presenter so its existing browser code
and physical-control semantics remain unchanged. `/api/platform-release` is
the exception: it reports hosting revision/activation metadata separately from
native `/api/release`. The hosting revision does not claim to be the native
binary revision or a new game release.

Run `node prototype/platform-server/server.mjs` with the existing Node runtime.
The dependency-free gateway binds loopback only. Configuration:

| Environment variable | Meaning/default |
| --- | --- |
| `PORT` | Loopback gateway port,4180 |
| `CRITTER_PLATFORM_HOST` | Allowed external hostname,`critterlab.basicberry.com` |
| `CRITTER_PLATFORM_REVISION` | Exact pushed hosting source revision; reports `unrecorded` if unset |
| `CRITTER_PLATFORM_ACTIVATED_AT` | Activation timestamp; null if unset |

Keep the existing workbench and native presenter as independent processes.
The gateway starts neither and supplies no updater, TLS tunnel or provider.
A workbench outage returns a local502 without interrupting website/native
routes. The existing trusted local TLS frontend must supply the original Host
and `X-Forwarded-Proto`. Native Authorization, Host, Origin, forwarded scheme,
authentication response and CSP pass through. Only the native HTML's exact
`href="/style.css"` and `src="/app.js"` attributes are rewritten to `/sandbox/`;
changed/missing attributes fail the mount explicitly.

Workbench requests validate original Host/scheme/Origin before translating
Host/Origin to the fixed loopback service. Mutations require same-origin Origin;
no CORS or arbitrary proxy destination is exposed. Workbench ingress stays
64KiB, including streamed bodies. Upstream requests have a30-second deadline;
incoming header/body and idle timeouts are finite. Static paths stay within
the real website directory, rejecting traversal/symlink escape/directories;
the repository is never a static root. Website scripts/styles are self-only.
The workbench CSP permits Mantine inline styles and local blob/data images,
while retaining self-only scripts and no remote scripting/inline-script bypass.

## Mount-aware workbench build and browser retention

Build the existing workbench with `CRITTER_BENCH_BASE=/genome/` for this mount.
Its API helper uses Vite `BASE_URL`; the preserved legacy page uses relative
assets/API paths. Leaving the variable unset keeps the standalone `/` build.
No genome, construction, prompt, game rule or native rendering profile changes.

Browser saves belong to an origin, not a server source directory. Existing
loopback workbench saves and IndexedDB pet proposals do not automatically move
to HTTPS. Export/import exact creature/draft recipes through the existing UI;
keep native bitmap plus linked metadata exports for returned pet proposals,
which currently have no image-import service. `/genome/` on the public origin
uses that origin's browser storage; the gateway neither copies nor evicts it.
Native saved worlds remain owned by the separately deployed presenter.

Source integration is not live deployment evidence. Integration review,
exact-revision build/activation and actual public-origin use remain separate
delivery gates. No additional tests or CI jobs are introduced; hosting uses
the existing runtimes and host.
