import { defineConfig } from 'vite'
import tailwindcss from '@tailwindcss/vite'

export default defineConfig({
  // Relative asset paths, so the build also works from a subfolder (e.g. itch.io).
  base: './',
  plugins: [
    tailwindcss(),
  ],
})