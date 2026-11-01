export function statusBadge(status: string): string {
  const cls = status === 'succeeded' ? 'badge-success'
    : status === 'failed' ? 'badge-danger'
    : status === 'cancelled' ? 'badge-warning'
    : 'badge-muted';
  return `<span class="badge ${cls}">${status}</span>`;
}

export function emptyState(message: string): string {
  return `<p class="empty">${message}</p>`;
}
