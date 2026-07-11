require('dotenv').config();

const express = require('express');
const http = require('http');
const { WebSocketServer } = require('ws');
const path = require('path');
const fs = require('fs');

const ChannelManager = require('./channel-manager');
const setupWsHandler = require('./ws-handler');

const PORT = process.env.PORT || 3000;
const DEVICE_TOKEN = process.env.DEVICE_TOKEN || 'esp32-secret-token-change-me';
const HLS_DIR = path.resolve(process.env.HLS_DIR || './hls');

// HLS dizini oluştur
fs.mkdirSync(HLS_DIR, { recursive: true });

const app = express();
const server = http.createServer(app);

// Static dosyalar (dashboard)
app.use(express.static(path.join(__dirname, '../public')));

// HLS dosyaları - CORS header'ları ile
app.use('/hls', (req, res, next) => {
    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Cache-Control', 'no-cache');
    if (req.path.endsWith('.m3u8')) {
        res.setHeader('Content-Type', 'application/vnd.apple.mpegurl');
    } else if (req.path.endsWith('.ts')) {
        res.setHeader('Content-Type', 'video/mp2t');
    }
    next();
}, express.static(HLS_DIR));

// Channel Manager
const channelManager = new ChannelManager(HLS_DIR);

// REST API
app.get('/api/channels', (req, res) => {
    res.json({
        count: channelManager.size,
        channels: channelManager.getChannelList()
    });
});

app.get('/api/channels/:id', (req, res) => {
    const ch = channelManager.getChannel(req.params.id);
    if (!ch) return res.status(404).json({ error: 'Kanal bulunamadı' });
    res.json(ch);
});

app.get('/api/stats', (req, res) => {
    const channels = channelManager.getChannelList();
    const totalFrames = channels.reduce((sum, ch) => sum + ch.frameCount, 0);
    res.json({
        activeChannels: channelManager.size,
        totalFrames,
        uptime: process.uptime()
    });
});

// WebSocket sunucu - /ws path'inde
const wss = new WebSocketServer({ server, path: '/ws' });
setupWsHandler(wss, channelManager, DEVICE_TOKEN);

// Periyodik temizlik (her 30 saniye)
setInterval(() => channelManager.cleanup(), 30000);

server.listen(PORT, () => {
    console.log(`\n=== ESP32S3-Algorhythm Server ===`);
    console.log(`HTTP:      http://localhost:${PORT}`);
    console.log(`WebSocket: ws://localhost:${PORT}/ws`);
    console.log(`HLS dir:   ${HLS_DIR}`);
    console.log(`Token:     ${DEVICE_TOKEN.substring(0, 8)}...`);
    console.log(`================================\n`);
});
