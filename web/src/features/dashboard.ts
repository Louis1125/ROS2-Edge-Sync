import { api, SyncJob } from '../shared/api/client';
import { emptyState, escapeHtml, progressBar, setUpdated, statusBadge } from '../shared/ui';

function kpi(label: string, value: string, sub: string, tone: string): string {
  return `<div class="kpi kpi-${tone}">
    <span class="kpi-label">${escapeHtml(label)}</span>
    <span class="kpi-value">${escapeHtml(value)}</span>
    <span class="kpi-sub">${escapeHtml(sub)}</span>
  </div>`;
}

function jobCard(j: SyncJob): string {
  return `<a class="job-card" href="#sync-jobs">
    <div class="job-card-top">
      <span class="job-artifact">${escapeHtml(j.artifact || '—')}</span>
      ${statusBadge(j.status)}
    </div>
    <div class="job-card-meta">${escapeHtml(j.sourceRobot)} → ${escapeHtml(j.targetRobot)}</div>
    ${progressBar(j.transferredBytes, j.totalBytes, j.status)}
  </a>`;
}

export async function renderDashboard(): Promise<string> {
  const [robots, jobs] = await Promise.all([api.robots(), api.syncJobs()]);
  const online = robots.filter((r) => r.status === 'online').length;
  const total = robots.length;
  const active = jobs.filter((j) => !['succeeded', 'failed', 'cancelled'].includes(j.status)).length;
  const failed = jobs.filter((j) => j.status === 'failed').length;
  const recent = [...jobs].reverse().slice(0, 6);

  return `<div class="page-head">
      <div>
        <h2>Fleet Overview</h2>
        <p class="muted">Health and activity across your ROS 2 fleet.</p>
      </div>
      <span class="live-tag" id="dashboard-updated">…</span>
    </div>
    <div class="kpi-grid">
      ${kpi('Robots', `${online}/${total}`, total ? 'online' : 'none registered', total === 0 ? 'muted' : online === total ? 'success' : 'info')}
      ${kpi('Active jobs', String(active), 'in progress', active ? 'info' : 'muted')}
      ${kpi('Failed jobs', String(failed), failed ? 'needs attention' : 'all clear', failed ? 'danger' : 'success')}
    </div>
    <section class="panel">
      <header class="panel-head">
        <h3>Recent jobs</h3>
        <a class="link" href="#sync-jobs">View all</a>
      </header>
      ${recent.length ? `<div class="job-list">${recent.map(jobCard).join('')}</div>` : emptyState('No sync jobs yet. Create one from the Sync Jobs page.')}
    </section>`;
}

export function mountDashboard(root: HTMLElement): () => void {
  setUpdated(root, 'dashboard-updated');
  const id = window.setInterval(async () => {
    try {
      root.innerHTML = await renderDashboard();
      setUpdated(root, 'dashboard-updated');
    } catch {
      /* keep last good render on a transient failure */
    }
  }, 5000);
  return () => window.clearInterval(id);
}
