import { api, getAdminToken, Robot, setAdminToken, SyncJob } from '../shared/api/client';
import { emptyState, escapeHtml, progressBar, setUpdated, statusBadge, truncate } from '../shared/ui';

function jobRow(j: SyncJob): string {
  const err = j.error
    ? `<span class="error-text" title="${escapeHtml(j.error)}">${escapeHtml(truncate(j.error, 48))}</span>`
    : '<span class="muted">—</span>';
  return `<tr>
    <td><code>${escapeHtml(j.jobId)}</code></td>
    <td><span class="route">${escapeHtml(j.sourceRobot)} <span class="arrow" aria-hidden="true">→</span> ${escapeHtml(j.targetRobot)}</span></td>
    <td>${escapeHtml(j.artifact || '—')}</td>
    <td class="cell-progress">${progressBar(j.transferredBytes, j.totalBytes, j.status)}</td>
    <td>${statusBadge(j.status)}</td>
    <td>${err}</td>
  </tr>`;
}

function jobsTable(jobs: SyncJob[]): string {
  if (!jobs.length) return emptyState('No sync jobs yet.');
  return `<div class="table-wrap"><table class="table">
    <thead><tr><th>Job</th><th>Route</th><th>Artifact</th><th>Progress</th><th>Status</th><th>Error</th></tr></thead>
    <tbody>${jobs.map(jobRow).join('')}</tbody>
  </table></div>`;
}

export async function renderSyncJobs(): Promise<string> {
  const jobs = await api.syncJobs();
  return `<div class="page-head">
      <div>
        <h2>Sync Jobs</h2>
        <p class="muted">Deploy artifacts from one robot to another.</p>
      </div>
      <span class="live-tag" id="jobs-updated">…</span>
    </div>
    <section class="panel">
      <header class="panel-head"><h3>New deployment</h3></header>
      <form id="deploy-form" class="form-grid">
        <div class="field">
          <label for="deploy-source">Source robot</label>
          <select name="source" id="deploy-source" required><option value="">— select a robot —</option></select>
        </div>
        <div class="field">
          <label for="deploy-target">Destination robot</label>
          <select name="target" id="deploy-target" required><option value="">— select a robot —</option></select>
        </div>
        <div class="field">
          <label for="deploy-artifact">Artifact ID</label>
          <input name="artifact" id="deploy-artifact" required placeholder="map-1" autocomplete="off" />
        </div>
        <div class="form-actions">
          <button type="submit" class="btn btn-primary">Deploy</button>
        </div>
        <p id="deploy-error" class="error-text" role="alert"></p>
        <div id="job-live" class="job-live" aria-live="polite"></div>
      </form>
      <details class="token-box">
        <summary>Admin token</summary>
        <p class="muted">Required only when the control plane has <code>EDGE_SYNC_ADMIN_TOKEN</code> set. Stored in this browser.</p>
        <input id="admin-token" type="password" placeholder="Bearer token" autocomplete="off" />
      </details>
    </section>
    <section class="panel">
      <header class="panel-head"><h3>History</h3></header>
      <div id="jobs-container">${jobsTable(jobs)}</div>
    </section>`;
}

function populateSelect(select: HTMLSelectElement | null, robots: Robot[], keep: string): void {
  if (!select) return;
  select.innerHTML = '<option value="">— select a robot —</option>';
  for (const r of robots) select.add(new Option(r.robotId, r.robotId));
  if (keep) select.value = keep;
}

export function mountSyncJobs(root: HTMLElement): () => void {
  let robots: Robot[] = [];
  let tracking: number | undefined;

  const refreshTable = async (): Promise<void> => {
    try {
      const jobs = await api.syncJobs();
      const container = root.querySelector<HTMLElement>('#jobs-container');
      if (container) container.innerHTML = jobsTable(jobs);
      setUpdated(root, 'jobs-updated');
    } catch {
      /* keep last good table */
    }
  };

  const refreshOptions = async (): Promise<void> => {
    try {
      robots = await api.robots();
      const source = root.querySelector<HTMLSelectElement>('#deploy-source');
      const target = root.querySelector<HTMLSelectElement>('#deploy-target');
      populateSelect(source, robots, source?.value ?? '');
      populateSelect(target, robots, target?.value ?? '');
    } catch {
      /* form still works with previously known robots */
    }
  };

  const trackJob = (jobId: string, live: HTMLElement | null): void => {
    if (tracking) window.clearInterval(tracking);
    let attempts = 0;
    tracking = window.setInterval(async () => {
      try {
        const job = await api.syncJob(jobId);
        if (live) {
          live.innerHTML = `${statusBadge(job.status)} Job <strong>${escapeHtml(jobId)}</strong>${job.error ? ` — ${escapeHtml(job.error)}` : ''}`;
          if (['succeeded', 'failed', 'cancelled'].includes(job.status)) {
            window.clearInterval(tracking);
            tracking = undefined;
          }
        }
        await refreshTable();
        if (++attempts > 120) {
          window.clearInterval(tracking);
          tracking = undefined;
        }
      } catch {
        if (++attempts > 120) {
          window.clearInterval(tracking);
          tracking = undefined;
          if (live) live.innerHTML = '<span class="error-text">Lost track of the job.</span>';
        }
      }
    }, 1500);
  };

  const token = root.querySelector<HTMLInputElement>('#admin-token');
  if (token) {
    token.value = getAdminToken();
    token.addEventListener('change', () => setAdminToken(token.value.trim()));
  }

  root.querySelector('#deploy-form')?.addEventListener('submit', async (e) => {
    e.preventDefault();
    const form = e.currentTarget as HTMLFormElement;
    const err = root.querySelector<HTMLElement>('#deploy-error');
    const live = root.querySelector<HTMLElement>('#job-live');
    const data = new FormData(form);
    const source = String(data.get('source') ?? '');
    const target = String(data.get('target') ?? '');
    const artifact = String(data.get('artifact') ?? '').trim();
    if (err) err.textContent = '';
    if (!source || !target || !artifact) {
      if (err) err.textContent = 'Choose a source and destination robot, and enter an artifact ID.';
      return;
    }
    const button = form.querySelector<HTMLButtonElement>('button[type="submit"]');
    if (button) button.disabled = true;
    try {
      const job = await api.createSyncJob(source, target, artifact);
      if (live) live.innerHTML = `<span class="badge badge-info">Deploying</span> Job <strong>${escapeHtml(job.jobId)}</strong> queued.`;
      trackJob(job.jobId, live);
      await refreshTable();
    } catch (ex) {
      const msg = ex instanceof Error ? ex.message : String(ex);
      if (err) {
        err.textContent = msg.includes('unauthorized')
          ? 'Unauthorized — add your admin token below and retry.'
          : msg;
      }
    } finally {
      if (button) button.disabled = false;
    }
  });

  void refreshOptions();
  void refreshTable();
  const id = window.setInterval(() => {
    void refreshOptions();
    void refreshTable();
  }, 5000);

  return () => {
    window.clearInterval(id);
    if (tracking) window.clearInterval(tracking);
  };
}
