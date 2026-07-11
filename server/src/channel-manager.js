const FFmpegEncoder = require('./ffmpeg-encoder');

class ChannelManager {
    constructor(hlsDir) {
        this.hlsDir = hlsDir;
        this.channels = new Map(); // channelId → { encoder, ws, chipId, ... }
    }

    createChannel(ws, chipId, resolution, generator) {
        // Aynı chip_id varsa eski kanalı kapat
        for (const [id, ch] of this.channels) {
            if (ch.chipId === chipId) {
                console.log(`[Channel] ${chipId} zaten var, eski kanal kapatılıyor`);
                this.removeChannel(id);
                break;
            }
        }

        const channelId = chipId; // chip_id'yi kanal ID olarak kullan
        const [w, h] = (resolution || '320x240').split('x').map(Number);

        const encoder = new FFmpegEncoder(channelId, this.hlsDir, {
            width: w, height: h, fps: 10
        });
        encoder.start();

        const channel = {
            channelId,
            chipId,
            ws,
            encoder,
            generator: generator || 'unknown',
            resolution: resolution || '320x240',
            connectedAt: Date.now(),
            lastFrame: Date.now(),
            frameCount: 0
        };

        this.channels.set(channelId, channel);
        console.log(`[Channel] Oluşturuldu: ${channelId} (${generator})`);
        return channelId;
    }

    writeFrame(channelId, jpegData) {
        const ch = this.channels.get(channelId);
        if (!ch) return false;
        ch.lastFrame = Date.now();
        ch.frameCount++;
        return ch.encoder.writeFrame(jpegData);
    }

    updateGenerator(channelId, generator) {
        const ch = this.channels.get(channelId);
        if (ch) ch.generator = generator;
    }

    removeChannel(channelId) {
        const ch = this.channels.get(channelId);
        if (!ch) return;
        ch.encoder.stop();
        this.channels.delete(channelId);
        console.log(`[Channel] Kaldırıldı: ${channelId}`);
    }

    removeByWs(ws) {
        for (const [id, ch] of this.channels) {
            if (ch.ws === ws) {
                this.removeChannel(id);
                return id;
            }
        }
        return null;
    }

    findByWs(ws) {
        for (const [id, ch] of this.channels) {
            if (ch.ws === ws) return id;
        }
        return null;
    }

    getChannelList() {
        const list = [];
        for (const [id, ch] of this.channels) {
            list.push({
                channelId: ch.channelId,
                chipId: ch.chipId,
                generator: ch.generator,
                resolution: ch.resolution,
                connectedAt: ch.connectedAt,
                lastFrame: ch.lastFrame,
                frameCount: ch.frameCount,
                uptime: Date.now() - ch.connectedAt
            });
        }
        return list;
    }

    getChannel(channelId) {
        const ch = this.channels.get(channelId);
        if (!ch) return null;
        return {
            channelId: ch.channelId,
            chipId: ch.chipId,
            generator: ch.generator,
            resolution: ch.resolution,
            connectedAt: ch.connectedAt,
            frameCount: ch.frameCount,
            uptime: Date.now() - ch.connectedAt
        };
    }

    // Ölü kanalları temizle (30 saniye frame gelmezse)
    cleanup() {
        const now = Date.now();
        for (const [id, ch] of this.channels) {
            if (now - ch.lastFrame > 30000) {
                console.log(`[Channel] ${id} timeout, temizleniyor`);
                this.removeChannel(id);
            }
        }
    }

    get size() {
        return this.channels.size;
    }
}

module.exports = ChannelManager;
