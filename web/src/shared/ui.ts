export function escapeHtml(value: unknown): string {
  return String(value ?? '')
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;')
    .replace(/"/g, '&quot;')
    .replace(/'/g, '&#39;');
}

export function formatBytes(n: number): string {
  if (!Number.isFinite(n) || n <= 0) return '0 B';
  const units = ['B', 'KB', 'MB', 'GB', 'TB'];
  let value = n;
  let i = 0;
  while (value >= 1024 && i < units.length - 1) {
    value /= 1024;
    i += 1;
  }
  const text = i === 0 || value >= 100 ? String(Math.round(value)) : value.toFixed(1);
  return `${text} ${units[i]}`;
}

export function timeAgo(epochMs: number, now = Date.now()): string {
  const s = Math.max(0, Math.floor((now - epochMs) / 1000));
  if (s < 10) return 'just now';
  if (s < 60) return `${s}s ago`;
  const m = Math.floor(s / 60);
  if (m < 60) return `${m}m ago`;
  const h = Math.floor(m / 60);
  if (h < 24) return `${h}h ago`;
  const d = Math.floor(h / 24);
  if (d < 7) return `${d}d ago`;
  return new Date(epochMs).toLocaleDateString();
}

const TONE: Record<string, string> = {
  online: 'success',
  succeeded: 'success',
  running: 'info',
  dispatched: 'info',
  acknowledged: 'info',
  queued: 'warning',
  cancelled: 'warning',
  offline: 'danger',
  failed: 'danger',
};

export function statusBadge(status: string): string {
  const tone = TONE[status] ?? 'muted';
  return `<span class="badge badge-${tone}"><span class="dot" aria-hidden="true"></span>${escapeHtml(status)}</span>`;
}

export function progressBar(transferred: number, total: number, status: string): string {
  const terminal = ['succeeded', 'failed', 'cancelled'].includes(status);
  const known = total > 0;
  const pct = known
    ? Math.min(100, Math.max(0, Math.round((transferred / total) * 100)))
    : terminal
      ? 100
      : 0;
  const cls = terminal
    ? `is-${status === 'succeeded' ? 'success' : status === 'failed' ? 'danger' : 'muted'}`
    : 'is-info';
  const label = known
    ? `${formatBytes(transferred)} / ${formatBytes(total)}`
    : terminal
      ? status
      : '—';
  return `<div class="progress-row">
    <div class="progress ${!known && !terminal ? 'is-indeterminate' : ''}" role="progressbar" aria-valuemin="0" aria-valuemax="100" aria-valuenow="${pct}">
      <div class="progress-bar ${cls}" style="width:${pct}%"></div>
    </div>
    <span class="progress-label">${escapeHtml(label)}</span>
  </div>`;
}

export function emptyState(message: string): string {
  return `<div class="empty"><p>${escapeHtml(message)}</p></div>`;
}

export function truncate(value: string, max: number): string {
  return value.length > max ? `${value.slice(0, max - 1)}…` : value;
}

export function setUpdated(root: HTMLElement, id: string): void {
  const el = root.querySelector<HTMLElement>(`#${id}`);
  if (el) el.textContent = `Updated ${new Date().toLocaleTimeString()}`;
}
