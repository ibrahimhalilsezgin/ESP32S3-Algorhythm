const grid = document.getElementById('grid');
const emptyState = document.getElementById('emptyState');
const channelCountEl = document.getElementById('channelCount');
const totalFramesEl = document.getElementById('totalFrames');
const modal = document.getElementById('modal');
const modalVideo = document.getElementById('modalVideo');
const modalInfo = document.getElementById('modalInfo');

let hlsInstances = {};  // channelId → Hls instance
let modalHls = null;

function formatUptime(ms) {
    const s = Math.floor(ms / 1000);
    const m = Math.floor(s / 60);
    const h = Math.floor(m / 60);
    if (h > 0) return `${h}s ${m % 60}dk`;
    if (m > 0) return `${m}dk ${s % 60}sn`;
    return `${s}sn`;
}

async function fetchChannels() {
    try {
        const res = await fetch('/api/channels');
        const data = await res.json();
        updateGrid(data.channels);
        channelCountEl.textContent = data.count;

        const statsRes = await fetch('/api/stats');
        const stats = await statsRes.json();
        totalFramesEl.textContent = stats.totalFrames.toLocaleString();
    } catch (e) {
        console.error('API hatası:', e);
    }
}

function updateGrid(channels) {
    if (channels.length === 0) {
        grid.innerHTML = '';
        grid.appendChild(emptyState);
        emptyState.classList.remove('hidden');
        // Eski HLS instance'ları temizle
        Object.values(hlsInstances).forEach(h => h.destroy());
        hlsInstances = {};
        return;
    }

    emptyState.classList.add('hidden');

    const existingIds = new Set();
    channels.forEach(ch => existingIds.add(ch.channelId));

    // Eski kartları kaldır
    const cards = grid.querySelectorAll('.channel-card');
    cards.forEach(card => {
        if (!existingIds.has(card.dataset.channelId)) {
            if (hlsInstances[card.dataset.channelId]) {
                hlsInstances[card.dataset.channelId].destroy();
                delete hlsInstances[card.dataset.channelId];
            }
            card.remove();
        }
    });

    // Kanalları güncelle/ekle
    channels.forEach(ch => {
        let card = grid.querySelector(`[data-channel-id="${ch.channelId}"]`);
        if (!card) {
            card = createChannelCard(ch);
            grid.appendChild(card);
            attachHls(ch.channelId, card.querySelector('video'));
        }
        // Bilgileri güncelle
        const genEl = card.querySelector('.generator');
        if (genEl) genEl.textContent = ch.generator;
        const framesEl = card.querySelector('.frames');
        if (framesEl) framesEl.textContent = `${ch.frameCount} frame · ${formatUptime(ch.uptime)}`;
    });
}

function createChannelCard(ch) {
    const card = document.createElement('div');
    card.className = 'channel-card';
    card.dataset.channelId = ch.channelId;
    card.innerHTML = `
        <video autoplay muted playsinline></video>
        <div class="channel-info">
            <div>
                <span class="live-badge">CANLI</span>
                <span class="chip-id">${ch.chipId}</span>
            </div>
            <span class="generator">${ch.generator}</span>
        </div>
        <div class="channel-info">
            <span class="frames">${ch.frameCount} frame · ${formatUptime(ch.uptime)}</span>
        </div>
    `;
    card.addEventListener('click', () => openModal(ch.channelId, ch));
    return card;
}

function attachHls(channelId, videoEl) {
    const url = `/hls/${channelId}/stream.m3u8`;

    if (Hls.isSupported()) {
        const hls = new Hls({
            liveDurationInfinity: true,
            enableWorker: false,
            lowLatencyMode: true,
            backBufferLength: 0
        });
        hls.loadSource(url);
        hls.attachMedia(videoEl);
        hls.on(Hls.Events.ERROR, (event, data) => {
            if (data.fatal) {
                // Yeniden dene
                setTimeout(() => {
                    hls.loadSource(url);
                    hls.attachMedia(videoEl);
                }, 3000);
            }
        });
        hlsInstances[channelId] = hls;
    } else if (videoEl.canPlayType('application/vnd.apple.mpegurl')) {
        // Safari native HLS
        videoEl.src = url;
    }
}

function openModal(channelId, ch) {
    modal.classList.remove('hidden');
    const url = `/hls/${channelId}/stream.m3u8`;

    if (modalHls) {
        modalHls.destroy();
    }

    if (Hls.isSupported()) {
        modalHls = new Hls({
            liveDurationInfinity: true,
            lowLatencyMode: true,
            backBufferLength: 0
        });
        modalHls.loadSource(url);
        modalHls.attachMedia(modalVideo);
    } else {
        modalVideo.src = url;
    }

    modalInfo.innerHTML = `
        <strong>${ch.chipId}</strong> · ${ch.generator} · ${ch.resolution} ·
        ${ch.frameCount} frame · ${formatUptime(ch.uptime)}
    `;
}

function closeModal() {
    modal.classList.add('hidden');
    if (modalHls) {
        modalHls.destroy();
        modalHls = null;
    }
    modalVideo.src = '';
}

// ESC ile modal kapat
document.addEventListener('keydown', (e) => {
    if (e.key === 'Escape') closeModal();
});

// Modal dışına tıklayınca kapat
modal.addEventListener('click', (e) => {
    if (e.target === modal) closeModal();
});

// Her 5 saniyede güncelle
fetchChannels();
setInterval(fetchChannels, 5000);
