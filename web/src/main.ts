import { renderDashboard, mountDashboard } from './features/dashboard';
import { renderRobots, mountRobots } from './features/robots';
import { renderSyncJobs, mountSyncJobs } from './features/sync-jobs';
import { escapeHtml } from './shared/ui';
import { initTheme } from './shared/theme';

interface Page {
  title: string;
  render: () => Promise<string>;
  mount?: (root: HTMLElement) => void | (() => void);
}

const routes: Record<string, Page> = {
  dashboard: { title: 'Dashboard', render: renderDashboard, mount: mountDashboard },
  robots: { title: 'Robots', render: renderRobots, mount: mountRobots },
  'sync-jobs': { title: 'Sync Jobs', render: renderSyncJobs, mount: mountSyncJobs },
};

const view = document.getElementById('view')!;
let unmount: (() => void) | undefined;

function resolveRoute(): string {
  const raw = (window.location.hash.replace(/^#\/?/, '') || 'dashboard').split('?')[0];
  if (raw === 'overview') return 'dashboard';
  return routes[raw] ? raw : 'dashboard';
}

function setActiveNav(route: string): void {
  document.querySelectorAll<HTMLElement>('[data-route]').forEach((el) => {
    const active = el.dataset.route === route;
    el.classList.toggle('active', active);
    if (active) el.setAttribute('aria-current', 'page');
    else el.removeAttribute('aria-current');
  });
}

function loadingState(): string {
  return `<div class="page-head">
      <div><h2>Loading…</h2><p class="muted">Fetching fleet data.</p></div>
    </div>
    <div class="skeleton" style="height:140px"></div>
    <div class="skeleton" style="height:240px"></div>`;
}

function errorState(message: string): string {
  return `<div class="error-banner" role="alert"><strong>Something went wrong.</strong>${escapeHtml(message)}</div>`;
}

async function load(route: string): Promise<void> {
  unmount?.();
  unmount = undefined;
  const page = routes[route] ?? routes.dashboard;
  setActiveNav(route);
  document.title = `${page.title} · ROS2-Edge-Sync`;
  view.innerHTML = loadingState();
  try {
    view.innerHTML = await page.render();
    const cleanup = page.mount?.(view);
    unmount = typeof cleanup === 'function' ? cleanup : undefined;
  } catch (err) {
    view.innerHTML = errorState(err instanceof Error ? err.message : String(err));
  }
}

window.addEventListener('hashchange', () => void load(resolveRoute()));
initTheme();
void load(resolveRoute());
