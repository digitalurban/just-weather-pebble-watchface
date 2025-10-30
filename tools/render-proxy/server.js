// Render-compatible proxy for Open-Meteo
// Deploy this repository to Render (connect to GitHub & create a Web Service).
// Routes:
//  GET /weather?lat=<lat>&lon=<lon>
//  GET /reverse-geocode?lat=<lat>&lon=<lon>

const express = require('express');
const fetch = require('node-fetch');
const app = express();
const PORT = process.env.PORT || 3000;

app.use(function(req, res, next) {
  res.header('Access-Control-Allow-Origin', '*');
  res.header('Access-Control-Allow-Methods', 'GET');
  res.header('Access-Control-Allow-Headers', 'Content-Type');
  next();
});

async function fetchJson(url) {
  const res = await fetch(url, { timeout: 10000 });
  const text = await res.text();
  try { return JSON.parse(text); } catch (e) { return { _raw: text }; }
}

app.get('/weather', async (req, res) => {
  const lat = req.query.lat;
  const lon = req.query.lon;
  if (!lat || !lon) return res.status(400).json({ error: 'lat and lon required' });
  const url = `https://api.open-meteo.com/v1/forecast?latitude=${encodeURIComponent(lat)}&longitude=${encodeURIComponent(lon)}&hourly=surface_pressure,relativehumidity_2m,windspeed_10m,precipitation&current_weather=true&timezone=auto`;
  try {
    const j = await fetchJson(url);
    res.json(j);
  } catch (e) {
    console.error('weather proxy error', e);
    res.status(502).json({ error: 'proxy fetch failed', detail: String(e) });
  }
});

app.get('/reverse-geocode', async (req, res) => {
  const lat = req.query.lat;
  const lon = req.query.lon;
  if (!lat || !lon) return res.status(400).json({ error: 'lat and lon required' });
  const url = `https://geocoding-api.open-meteo.com/v1/reverse?latitude=${encodeURIComponent(lat)}&longitude=${encodeURIComponent(lon)}&count=1&language=en`;
  try {
    const j = await fetchJson(url);
    res.json(j);
  } catch (e) {
    console.error('reverse proxy error', e);
    res.status(502).json({ error: 'proxy fetch failed', detail: String(e) });
  }
});

app.get('/', (req, res) => res.send('Pebble Render proxy running'));

app.listen(PORT, () => console.log('Render proxy listening on port', PORT));
