import { api, Robot, SyncJob } from '../shared/api/client';
import { statusBadge, emptyState } from '../shared/components/status';

function jobRow(j: SyncJob): string {
  return `<tr>
    <td>${j.jobId}</td><td>${j.sourceRobot} → ${j.targetRobot}</td>
    <td>${j.artifact}</td><td>${statusBadge(j.status)}</td>
    <td>${j.error ? `<span class="error" title="${j.error}">${j.error}</span>` : '—'}</td>
  </tr>`;
}

export async function renderSyncJobs(): Promise<string> {
  const jobs = await api.syncJobs();
  const rows = jobs.map(jobRow).join('');
  return `
    <h2>Sync Jobs</h2>
    <details class="wizard">
      <summary>Create a deployment</summary>
      <form id="deploy-form">
        <label>Source robot
          <select name="source" id="deploy-source" required><option value="">— select —</option></select>
        </label>
        <label>Destination robot
          <select name="target" id="deploy-target" required><option value="">— select —</option></select>
        </label>
        <label>Artifact ID <input name="artifact" id="deploy-artifact" required placeholder="map-1" /></label>
        <button type="submit">Deploy</button>
        <p id="deploy-error" class="error" role="alert"></p>
      </form>
    </details>
    <div id="job-live" aria-live="polite"></div>
    ${jobs.length ? `<table><thead><tr><th>Job</th><th>Robots</th><th>Artifact</th><th>Status</th><th>Error</th></tr></thead><tbody>${rows}</tbody></table>` : emptyState('No jobs yet')}
  `;
}

export function mountSyncJobs(view: HTMLElement): void {
  void (async () => {
    const robots: Robot[] = await api.robots();
    const source = view.querySelector<HTMLSelectElement>('#deploy-source');
    const target = view.querySelector<HTMLSelectElement>('#deploy-target');
    for (const r of robots) {
      source?.add(new Option(r.robotId, r.robotId));
      target?.add(new Option(r.robotId, r.robotId));
    }
  })();

  view.querySelector('#deploy-form')?.addEventListener('submit', async (e) => {
    e.preventDefault();
    const err = view.querySelector<HTMLElement>('#deploy-error');
    const live = view.querySelector<HTMLElement>('#job-live');
    if (err) err.textContent = '';
    const source = (view.querySelector('#deploy-source') as HTMLSelectElement).value;
    const target = (view.querySelector('#deploy-target') as HTMLSelectElement).value;
    const artifact = (view.querySelector('#deploy-artifact') as HTMLInputElement).value.trim();
    try {
      const job = await api.createSyncJob(source, target, artifact);
      if (live) live.innerHTML = `<p>Job <strong>${job.jobId}</strong> created.</p>`;
      pollJob(job.jobId, live);
    } catch (ex) {
      if (err) err.textContent = String(ex);
    }
  });
}

function pollJob(jobId: string, live: HTMLElement | null): void {
  let attempts = 0;
  const timer = setInterval(async () => {
    try {
      const jobs = await api.syncJobs();
      const job = jobs.find((j) => j.jobId === jobId);
      if (!job) return;
      if (live) live.innerHTML = `<p>Job <strong>${jobId}</strong>: ${statusBadge(job.status)} ${job.error ? `— ${job.error}` : ''}</p>`;
      if (['succeeded', 'failed', 'cancelled'].includes(job.status) || ++attempts > 60) {
        clearInterval(timer);
      }
    } catch {
      clearInterval(timer);
      if (live) live.innerHTML = '<p class="error">Lost connection while tracking the job.</p>';
    }
  }, 1000);
}
