# Pebble HTTP proxy (Glitch template)

This is a tiny Node/Express proxy you can deploy to Glitch (or any small hosting service). It allows Pebble SDK v3 companion JS (which cannot make HTTPS requests) to fetch Open-Meteo data through simple HTTP endpoints.

Routes
- GET /weather?lat=<lat>&lon=<lon>
  - Proxies to Open-Meteo forecast endpoint and returns JSON.

- GET /reverse-geocode?lat=<lat>&lon=<lon>
  - Proxies to Open-Meteo reverse geocoding endpoint and returns JSON.

Deploy to Glitch
1. Create a new project on Glitch ("import from GitHub" or "new project -> hello-express").
2. Copy `package.json` and `server.js` into the project (replace existing files).
3. Add the `node-fetch` dependency (already in package.json). Glitch will install it automatically.
4. Start the project — Glitch will give you a URL like `https://my-pebble-proxy.glitch.me`.
5. You can use the same host over HTTP (Glitch supports both in preview) — use `http://your-project.glitch.me` in the Pebble companion.

Security notes
- This proxy is intentionally minimal and publicly accessible; don't add secrets to it.
- If you plan to publish widely, consider rate-limiting, caching, or adding an API key.

Usage
- In `src/pkjs/app.js` set `PROXY_BASE` to `http://your-project.glitch.me` and rebuild the Pebble app.

Example
`http://your-project.glitch.me/weather?lat=51.5074&lon=-0.1278`

