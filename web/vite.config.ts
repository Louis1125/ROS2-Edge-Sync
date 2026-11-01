import { defineConfig } from 'vite';

const apiTarget = process.env.VITE_API_TARGET || 'http://localhost:8080';

export default defineConfig({
  build: { outDir: 'dist', emptyOutDir: true },
  server: {
    proxy: {
      '/api': { target: apiTarget, changeOrigin: true },
      '/events': { target: apiTarget, changeOrigin: true },
    },
  },
});
