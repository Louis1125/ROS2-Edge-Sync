export interface Robot {
  robotId: string;
  name: string;
  status: string;
  runtimeVersion: string;
  lastSeenMs: number;
}

async function request<T>(path: string): Promise<T> {
  const response = await fetch(path);
  if (!response.ok) {
    throw new Error(`request failed: ${response.status}`);
  }
  return response.json() as Promise<T>;
}

export const api = {
  health: () => request<{ status: string }>('/api/v1/health'),
  robots: () => request<Robot[]>('/api/v1/robots'),
};
