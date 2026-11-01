CREATE TABLE IF NOT EXISTS robots (
  robot_id TEXT PRIMARY KEY,
  name TEXT NOT NULL DEFAULT '',
  status TEXT NOT NULL DEFAULT 'offline',
  runtime_version TEXT NOT NULL DEFAULT '',
  last_seen_ms BIGINT NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS artifacts (
  artifact_id TEXT PRIMARY KEY,
  version_major INTEGER NOT NULL DEFAULT 0,
  version_minor INTEGER NOT NULL DEFAULT 0,
  version_patch INTEGER NOT NULL DEFAULT 0,
  total_size BIGINT NOT NULL DEFAULT 0,
  artifact_hash TEXT NOT NULL DEFAULT ''
);

CREATE TABLE IF NOT EXISTS sync_jobs (
  job_id TEXT PRIMARY KEY,
  source_robot TEXT NOT NULL,
  target_robot TEXT NOT NULL,
  artifact_id TEXT NOT NULL,
  status TEXT NOT NULL DEFAULT 'queued',
  transferred_bytes BIGINT NOT NULL DEFAULT 0,
  total_bytes BIGINT NOT NULL DEFAULT 0,
  error TEXT NOT NULL DEFAULT ''
);

CREATE TABLE IF NOT EXISTS audit_events (
  id BIGSERIAL PRIMARY KEY,
  occurred_at BIGINT NOT NULL,
  actor TEXT NOT NULL,
  action TEXT NOT NULL,
  detail TEXT NOT NULL DEFAULT ''
);
