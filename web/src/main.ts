import { renderOverview } from './features/overview';
import { renderRobots } from './features/robots';
import { renderSyncJobs, mountSyncJobs } from './features/sync-jobs';

const view = document.getElementById('view')!;

const routes: Record<string, () => Promise<string>> = {
  overview: renderOverview,
  robots: renderRobots,
  'sync-jobs': renderSyncJobs,
};

async function refresh(): Promise<void> {
  const route = (location.hash.replace('#', '') || 'overview').replace('/', '');
  const render = routes[route] ?? renderOverview;
  view.innerHTML = '<p>Loading…</p>';
  try {
    view.innerHTML = await render();
    if (route === 'sync-jobs') mountSyncJobs(view);
  } catch (err) {
    view.innerHTML = `<p class="error">Failed to load: ${String(err)}</p>`;
  }
}

window.addEventListener('hashchange', () => void refresh());
void refresh();
