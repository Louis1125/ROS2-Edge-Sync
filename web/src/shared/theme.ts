export type Theme = 'light' | 'dark' | 'system';

const KEY = 'edge-sync-theme';
const ORDER: Theme[] = ['system', 'light', 'dark'];

export function currentTheme(): Theme {
  try {
    const saved = window.localStorage.getItem(KEY);
    if (saved === 'light' || saved === 'dark') return saved;
  } catch {
    /* ignore */
  }
  return 'system';
}

function label(t: Theme): string {
  return `Theme: ${t[0].toUpperCase()}${t.slice(1)}`;
}

function apply(t: Theme): void {
  const root = document.documentElement;
  if (t === 'system') root.removeAttribute('data-theme');
  else root.setAttribute('data-theme', t);

  try {
    if (t === 'system') window.localStorage.removeItem(KEY);
    else window.localStorage.setItem(KEY, t);
  } catch {
    /* ignore */
  }

  const btn = document.getElementById('theme-toggle');
  if (btn) {
    btn.textContent = label(t);
    btn.setAttribute('aria-label', label(t));
    btn.title = 'Click to switch theme';
  }
}

export function cycleTheme(): Theme {
  const next = ORDER[(ORDER.indexOf(currentTheme()) + 1) % ORDER.length];
  apply(next);
  return next;
}

export function initTheme(): void {
  apply(currentTheme());
  document.getElementById('theme-toggle')?.addEventListener('click', () => void cycleTheme());
}
