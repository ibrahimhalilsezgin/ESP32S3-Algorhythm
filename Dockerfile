FROM node:20-alpine

RUN apk add --no-cache ffmpeg

WORKDIR /app
COPY server/package*.json ./
RUN npm ci --production
COPY server/ ./

RUN mkdir -p hls

EXPOSE 3000

CMD ["node", "src/index.js"]
