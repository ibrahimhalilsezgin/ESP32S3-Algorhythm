const ChannelManager = require('./channel-manager');

function setupWsHandler(wss, channelManager, deviceToken) {
    wss.on('connection', (ws, req) => {
        const ip = req.socket.remoteAddress;
        console.log(`[WS] Yeni bağlantı: ${ip}`);

        let channelId = null;

        let frameCounter = 0;
        ws.on('message', (data, isBinary) => {
            if (isBinary) {
                // Binary = JPEG frame
                if (channelId) {
                    frameCounter++;
                    if (frameCounter % 30 === 0) {
                        console.log(`[WS] Frame alindi: ${data.length} bytes. Ilk 4 byte: ${data.slice(0, 4).toString('hex')}`);
                    }
                    channelManager.writeFrame(channelId, data);
                }
                return;
            }

            // Text = JSON mesaj
            try {
                const msg = JSON.parse(data.toString());

                switch (msg.type) {
                    case 'register':
                        // Token doğrula
                        if (msg.token !== deviceToken) {
                            ws.send(JSON.stringify({
                                type: 'error',
                                message: 'Geçersiz token'
                            }));
                            ws.close();
                            return;
                        }

                        channelId = channelManager.createChannel(
                            ws,
                            msg.chip_id,
                            msg.resolution,
                            msg.generator
                        );

                        ws.send(JSON.stringify({
                            type: 'registered',
                            channel_id: channelId
                        }));
                        console.log(`[WS] Kayıt: ${msg.chip_id} → kanal ${channelId}`);
                        break;

                    case 'generator_change':
                        if (channelId) {
                            channelManager.updateGenerator(channelId, msg.generator);
                            console.log(`[WS] ${channelId} generator: ${msg.generator}`);
                        }
                        break;

                    default:
                        console.log(`[WS] Bilinmeyen mesaj tipi: ${msg.type}`);
                }
            } catch (e) {
                console.error('[WS] JSON parse hatası:', e.message);
            }
        });

        ws.on('close', () => {
            console.log(`[WS] Bağlantı kapandı: ${ip}`);
            if (channelId) {
                // 30 saniye bekle, belki yeniden bağlanır
                setTimeout(() => {
                    const ch = channelManager.getChannel(channelId);
                    if (ch && Date.now() - ch.lastFrame > 25000) {
                        channelManager.removeChannel(channelId);
                    }
                }, 30000);
            }
        });

        ws.on('error', (err) => {
            console.error(`[WS] Hata: ${err.message}`);
        });
    });
}

module.exports = setupWsHandler;
