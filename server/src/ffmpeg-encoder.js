const { spawn } = require('child_process');
const fs = require('fs');
const path = require('path');

class FFmpegEncoder {
    constructor(channelId, hlsDir, options = {}) {
        this.channelId = channelId;
        this.hlsDir = path.join(hlsDir, channelId);
        this.fps = options.fps || 10;
        this.width = options.width || 320;
        this.height = options.height || 240;
        this.process = null;
        this.frameCount = 0;
    }

    start() {
        // HLS dizini oluştur
        fs.mkdirSync(this.hlsDir, { recursive: true });

        const outputPath = path.join(this.hlsDir, 'stream.m3u8');

        this.process = spawn('ffmpeg', [
            '-analyzeduration', '0',
            '-probesize', '32',
            '-f', 'image2pipe',
            '-vcodec', 'mjpeg',
            '-framerate', String(this.fps),
            '-i', 'pipe:0',
            // Upscale video to 1080p using Lanczos filtering (smooth & high quality)
            '-vf', 'scale=1920:1080:flags=lanczos',
            // Video codec
            '-c:v', 'libx264',
            '-preset', 'ultrafast',
            '-tune', 'zerolatency',
            '-pix_fmt', 'yuv420p',
            '-g', '10', // Keyframe interval
            // HLS output
            '-f', 'hls',
            '-hls_time', '1',
            '-hls_list_size', '3',
            '-hls_flags', 'delete_segments+append_list+split_by_time',
            '-hls_segment_filename', path.join(this.hlsDir, 'seg_%03d.ts'),
            outputPath
        ], {
            stdio: ['pipe', 'pipe', 'pipe']
        });

        this.process.stderr.on('data', (data) => {
            const msg = data.toString().trim();
            console.log(`[FFmpeg ${this.channelId}] ${msg}`);
        });

        this.process.on('close', (code) => {
            console.log(`[FFmpeg ${this.channelId}] Process kapandı, code: ${code}`);
            this.process = null;
        });

        this.process.on('error', (err) => {
            console.error(`[FFmpeg ${this.channelId}] Spawn hatası:`, err.message);
            this.process = null;
        });

        console.log(`[FFmpeg ${this.channelId}] Başlatıldı: ${this.width}x${this.height}@${this.fps}fps`);
    }

    writeFrame(jpegData) {
        if (!this.process || !this.process.stdin.writable) return false;
        try {
            this.process.stdin.write(jpegData);
            this.frameCount++;
            return true;
        } catch (e) {
            console.error(`[FFmpeg ${this.channelId}] Write hatası:`, e.message);
            return false;
        }
    }

    stop() {
        if (this.process) {
            try {
                this.process.stdin.end();
                // 5 saniye bekle, sonra kill
                setTimeout(() => {
                    if (this.process) {
                        this.process.kill('SIGKILL');
                        this.process = null;
                    }
                }, 5000);
            } catch (e) {
                if (this.process) {
                    this.process.kill('SIGKILL');
                    this.process = null;
                }
            }
        }
        // HLS dosyalarını temizle
        setTimeout(() => {
            try {
                fs.rmSync(this.hlsDir, { recursive: true, force: true });
                console.log(`[FFmpeg ${this.channelId}] HLS dosyaları temizlendi`);
            } catch (e) { /* zaten silinmiş */ }
        }, 10000);
    }

    get stats() {
        return {
            channelId: this.channelId,
            frameCount: this.frameCount,
            running: !!this.process
        };
    }
}

module.exports = FFmpegEncoder;
