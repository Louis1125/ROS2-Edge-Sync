export interface Robot {
  robotId: string;
  name: string;
  status: string;
  runtimeVersion: string;
  lastSeenMs: number;
}

export interface SyncJob {
  jobId: string;
  sourceRobot: string;
  targetRobot: string;
  artifact: string;
  status: string; // queued | dispatched | acknowledged | running | succeeded | failed | cancelled
  transferredBytes: number;
  totalBytes: number;
  error: string;
}

const TOKEN_KEY = 'edge-sync-admin-token';

export function getAdminToken(): string {
  try {
    return window.localStorage.getItem(TOKEN_KEY) ?? '';
  } catch {
    return '';
  }
}

export function setAdminToken(token: string): void {
  try {
    if (token) window.localStorage.setItem(TOKEN_KEY, token);
    else window.localStorage.removeItem(TOKEN_KEY);
  } catch {
    /* ignore */
  }
}

async function request<T>(path: string, init?: RequestInit): Promise<T> {
  const response = await fetch(path, init);
  if (!response.ok) {
    let message = `Request failed (${response.status})`;
    try {
      const body = (await response.json()) as { error?: string };
      if (body.error) message = body.error;
    } catch {
      /* non-JSON error */
    }
    throw new Error(message);
  }
  return (await response.json()) as T;
}

function authHeaders(): Record<string, string> {
  const token = getAdminToken();
  return token ? { Authorization: `Bearer ${token}` } : {};
}

export const api = {
  health: () => request<{ status: string }>('/api/v1/health'),
  robots: () => request<Robot[]>('/api/v1/robots'),
  syncJobs: () => request<SyncJob[]>('/api/v1/sync-jobs'),
  syncJob: (id: string) => request<SyncJob>(`/api/v1/sync-jobs/${encodeURIComponent(id)}`),
  createSyncJob: (source: string, target: string, artifact: string) =>
    request<SyncJob>('/api/v1/sync-jobs', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json', ...authHeaders() },
      body: JSON.stringify({ source_robot: source, target_robot: target, artifact_id: artifact }),
    }),
};
