# ESP32S3-Algorhythm

This project enables one or more **ESP32-S3** microcontrollers to generate real-time visuals using mathematical algorithms (Fractal, Cellular Automata, Noise, etc.), stream them to a Node.js server via WebSockets, and transcode the frames on the fly using FFmpeg into a low-latency HLS (HTTP Live Streaming) video stream playable on a web-based dashboard.

## System Architecture

```
 ┌──────────────────────┐  WebSocket  ┌───────────────────────────┐  HLS (.m3u8)  ┌──────────────────┐
 │  ESP32-S3 Device     │ ──────────→ │ Node.js Streaming Server  │ ────────────→ │ Web Dashboard    │
 │                      │ (Binary     │                           │               │ (HLS.js Player)  │
 │  - Algorithm Switch  │  JPEG)      │  - Connection Management  │               └──────────────────┘
 │  - RGB565 Framebuffer              │  - FFmpeg Transcoder      │
 │  - esp-camera JPEG                 │  - HLS Segmenter          │
 └──────────────────────┘             └───────────────────────────┘
```

## Features

- **Generative Art Engines:** 5 built-in mathematical visual generators running directly on the ESP32-S3 (Plasma wave effect, Mandelbrot Fractal Zoom, Perlin Noise flow field, Conway's Game of Life, and Symmetric Pixel Art).
- **Auto-Algorithm Switching:** The ESP32 rotates through algorithms periodically (e.g. every 30 seconds) and updates the server.
- **Hardware-Accelerated JPEG Compression:** Utilizes Espressif's optimized vector/SIMD `fmt2jpg` API inside the ESP32-S3 SDK to compress raw RGB frames instantly.
- **Low-Latency HLS:** Node.js pipes the raw MJPEG stream directly into FFmpeg configured with an ultra-fast H.264 profile and 1-second segment intervals to minimize broadcast delay.
- **Dynamic Web Dashboard:** A responsive web UI displaying a grid of all active ESP32 streams with metadata, using HLS.js for web playback.
- **Token-Based Authentication:** Basic handshake validation to prevent unauthorized devices from hijacking streams.

---

## Installation & Setup

### 1. Server Setup (Node.js)

The server requires **Node.js** and **FFmpeg** installed on the host system.

```bash
# Install dependencies
cd server
npm install

# Configure environment (Optional)
# You can edit server/.env to modify PORT, HLS_DIR, and the shared DEVICE_TOKEN.

# Start the server
npm start
```
*The web dashboard will be available at `http://localhost:3000`.*

### 2. ESP32-S3 Firmware Upload (PlatformIO)

1. Open `firmware/src/config.h` and configure:
   - `WIFI_SSID` and `WIFI_PASS` with your network credentials.
   - `WS_HOST` with your server's IP address (e.g., `"192.168.1.78"` if running locally).
   - `DEVICE_TOKEN` matching the token set in the server's `.env`.
2. Connect the ESP32-S3 to your computer.
3. Build and upload:
   ```bash
   cd firmware
   pio run -t upload
   ```

---

## Deploying with Docker

To deploy the streaming server on a VPS, use the pre-configured Docker Compose file:

```bash
# Start server in detached mode
docker-compose up -d
```

---

## Project Structure

```
├── firmware/                   # ESP32-S3 PlatformIO Project
│   ├── platformio.ini          # Library and build flags configuration
│   └── src/
│       ├── config.h            # WiFi & server credentials
│       ├── main.cpp            # Main loop & WebSocket client logic
│       ├── wifi_manager.h      # WiFi reconnection helper
│       └── generators/         # Visual generator classes (Plasma, Fractal, etc.)
├── server/                     # Node.js Streaming Server
│   ├── src/
│   │   ├── index.js            # Server entrypoint
│   │   ├── channel-manager.js  # Active channel lifecycle
│   │   ├── ffmpeg-encoder.js   # FFmpeg process spawner & pipe writer
│   │   └── ws-handler.js       # WebSocket packet router
│   └── public/                 # Web Dashboard Frontend
```
