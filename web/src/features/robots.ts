import { api, Robot } from '../shared/api/client';
import { statusBadge, emptyState } from '../shared/components/status';

export async function renderRobots(): Promise<string> {
  const robots = await api.robots();
  if (!robots.length) return `<h2>Robots</h2>${emptyState('No robots registered yet. Start a robot agent to connect.')}`;
  const rows = robots
    .map((r) => `<tr><td>${r.robotId}</td><td>${r.name || '—'}</td><td>${statusBadge(r.status)}</td><td>${r.runtimeVersion || '—'}</td></tr>`)
    .join('');
  return `<h2>Robots</h2><table><thead><tr><th>Robot</th><th>Name</th><th>Status</th><th>Version</th></tr></thead><tbody>${rows}</tbody></table>`;
}
