import { renderDashboard } from './features/dashboard/dashboard';

const view = document.getElementById('view')!;

async function refresh(): Promise<void> {
  try {
    view.innerHTML = await renderDashboard();
  } catch (err) {
    view.innerHTML = `<p class="error">${String(err)}</p>`;
  }
}

void refresh();
