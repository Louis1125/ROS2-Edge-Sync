# Frontend: build with Node 20, serve with nginx, proxy /api to the control plane.
FROM node:20-alpine AS build
WORKDIR /app
COPY web/package.json ./
RUN npm install --no-audit --no-fund
COPY web/ .
RUN npm run build

FROM nginx:alpine
COPY --from=build /app/dist /usr/share/nginx/html
