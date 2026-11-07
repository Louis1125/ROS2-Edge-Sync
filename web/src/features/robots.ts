import { api, Robot } from '../shared/api/client';
import { emptyState, escapeHtml, setUpdated, statusBadge, timeAgo } from '../shared/ui';

function row(r: Robot): string {
  return `<tr>
    <td><code>${escapeHtml(r.robotId)}</code></td>
    <td>${escapeHtml(r.name || '—')}</td>
    <td>${statusBadge(r.status)}</td>
    <td>${escapeHtml(r.runtimeVersion || '—')}</td>
    <td class="muted">${escapeHtml(timeAgo(r.lastSeenMs))}</td>
  </tr>`;
}

export async function renderRobots(): Promise<string> {
  const robots = await api.robots();
  const online = robots.filter((r) => r.status === 'online').length;
  return `<div class="page-head">
      <div>
        <h2>Robots</h2>
        <p class="muted">${robots.length ? `${online} of ${robots.length} online` : 'No robots registered yet.'}</p>
      </div>
      <span class="live-tag" id="robots-updated">…</span>
    </div>
    ${
      robots.length
        ? `<div class="table-wrap"><table class="table">
            <thead><tr><th>Robot</th><th>Name</th><th>Status</th><th>Runtime</th><th>Last seen</th></tr></thead>
            <tbody>${robots.map(row).join('')}</tbody>
          </table></div>`
        : emptyState('No robots registered yet. Start a robot agent to connect.')
    }`;
}

export function mountRobots(root: HTMLElement): () => void {
  setUpdated(root, 'robots-updated');
  const id = window.setInterval(async () => {
    try {
      root.innerHTML = await renderRobots();
      setUpdated(root, 'robots-updated');
    } catch {
      /* keep last good render */
    }
  }, 5000);
  return () => window.clearInterval(id);
}
