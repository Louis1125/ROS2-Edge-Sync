import { api } from '../../shared/api/client';

export async function renderDashboard(): Promise<string> {
  const robots = await api.robots();
  const online = robots.filter((r) => r.status === 'online').length;
  const rows = robots
    .map(
      (r) =>
        `<tr><td>${r.robotId}</td><td>${r.name}</td>` +
        `<td class="badge-${r.status}">${r.status}</td><td>${r.runtimeVersion}</td></tr>`,
    )
    .join('');
  return `
    <h2>Fleet Overview</h2>
    <p>${online} of ${robots.length} robots online</p>
    <table><thead><tr><th>Robot</th><th>Name</th><th>Status</th><th>Version</th></tr></thead>
    <tbody>${rows || '<tr><td colspan="4">No robots registered</td></tr>'}</tbody></table>`;
}
