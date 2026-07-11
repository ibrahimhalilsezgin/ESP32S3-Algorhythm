#pragma once

// WiFi
#define WIFI_SSID       "TurkNet1000Mbps_EDE71"
#define WIFI_PASS       "4GMnW8Yhcr21"

// Sunucu
#define WS_HOST         "192.168.1.78"
#define WS_PORT         3000
#define WS_PATH         "/ws"
#define DEVICE_TOKEN    "esp32-secret-token-change-me"

// Görüntü
#define FRAME_WIDTH     600
#define FRAME_HEIGHT    600
#define TARGET_FPS      10
#define JPEG_QUALITY    50  // 1-100, düşük = küçük dosya

// Generator
#define AUTO_SWITCH_INTERVAL_MS  30000  // 30 saniyede bir algoritma değiştir
