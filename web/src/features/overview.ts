import { api } from '../shared/api/client';

export async function renderOverview(): Promise<string> {
  const [robots, jobs] = await Promise.all([api.robots(), api.syncJobs()]);
  const online = robots.filter((r) => r.status === 'online').length;
  const active = jobs.filter((j) => !['succeeded', 'failed', 'cancelled'].includes(j.status)).length;
  const failed = jobs.filter((j) => j.status === 'failed').length;
  return `
    <h2>Fleet Overview</h2>
    <div class="stat-row">
      <div class="stat"><strong>${robots.length}</strong> robots (${online} online)</div>
      <div class="stat"><strong>${active}</strong> active jobs</div>
      <div class="stat"><strong>${failed}</strong> failed jobs</div>
    </div>
    <h3>Recent jobs</h3>
    ${jobs.length ? `<ul>${jobs.slice(0, 8).map((j) => `<li>${j.jobId} — ${j.status}</li>`).join('')}</ul>` : '<p class="empty">No jobs yet</p>'}
  `;
}
