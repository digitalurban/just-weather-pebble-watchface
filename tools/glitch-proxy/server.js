// Simple proxy for Open-Meteo and geocoding for Pebble companion apps
// Deploy this to Glitch (or any small Node host). Exposes two routes:
// GET /weather?lat=...&lon=...         -> proxied Open-Meteo forecast JSON
// GET /reverse-geocode?lat=...&lon=... -> proxied reverse-geocode JSON

const express = require('express');
const fetch = require('node-fetch');
const app = express();
const PORT = process.env.PORT || 3000;

// Helper to forward JSON responses and set permissive CORS
async function fetchJson(url) {
  const res = await fetch(url, { timeout: 10000 });
  const text = await res.text();
  // Try to parse JSON; if parsing fails, return text as { _raw: text }
  try {
    return JSON.parse(text);
  } catch (e) {
    return { _raw: text };
  }
}

app.use(function(req, res, next) {
  res.header('Access-Control-Allow-Origin', '*');
  res.header('Access-Control-Allow-Methods', 'GET');
  res.header('Access-Control-Allow-Headers', 'Content-Type');
  next();
});

app.get('/weather', async (req, res) => {
  const lat = req.query.lat;
  const lon = req.query.lon;
  if (!lat || !lon) return res.status(400).json({ error: 'lat and lon required' });
  // Build Open-Meteo URL (server-side HTTPS ok)
  const url = `https://api.open-meteo.com/v1/forecast?latitude=${encodeURIComponent(lat)}&longitude=${encodeURIComponent(lon)}&hourly=surface_pressure,relativehumidity_2m,windspeed_10m,precipitation&current_weather=true&timezone=auto`;
  try {
    const j = await fetchJson(url);
    return res.json(j);
  } catch (e) {
    console.error('weather fetch error', e);
    return res.status(502).json({ error: 'proxy fetch failed', detail: String(e) });
  }
});

app.get('/reverse-geocode', async (req, res) => {
  const lat = req.query.lat;
  const lon = req.query.lon;
  if (!lat || !lon) return res.status(400).json({ error: 'lat and lon required' });
  const url = `https://geocoding-api.open-meteo.com/v1/reverse?latitude=${encodeURIComponent(lat)}&longitude=${encodeURIComponent(lon)}&count=1&language=en`;
  try {
    const j = await fetchJson(url);
    return res.json(j);
  } catch (e) {
    console.error('reverse fetch error', e);
    return res.status(502).json({ error: 'proxy fetch failed', detail: String(e) });
  }
});

// Health
app.get('/', (req, res) => res.send('Pebble proxy running'));

app.listen(PORT, () => {
  console.log('Proxy listening on port', PORT);
});
