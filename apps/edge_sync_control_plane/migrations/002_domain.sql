-- Unified fleet domain. Artifact releases are immutable; deployments are
-- aggregate rollout operations; sync_jobs are per-robot executions.
CREATE TABLE IF NOT EXISTS robot_groups (
  group_id TEXT PRIMARY KEY,
  name TEXT NOT NULL DEFAULT ''
);

CREATE TABLE IF NOT EXISTS artifact_releases (
  release_id TEXT PRIMARY KEY,
  artifact_id TEXT NOT NULL,
  version_major INTEGER NOT NULL DEFAULT 0,
  version_minor INTEGER NOT NULL DEFAULT 0,
  version_patch INTEGER NOT NULL DEFAULT 0,
  total_size BIGINT NOT NULL DEFAULT 0,
  artifact_hash TEXT NOT NULL DEFAULT '',
  created_at BIGINT NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS deployments (
  deployment_id TEXT PRIMARY KEY,
  artifact_release_id TEXT NOT NULL,
  status TEXT NOT NULL DEFAULT 'queued',
  strategy TEXT NOT NULL DEFAULT 'all-at-once',
  created_at BIGINT NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS deployment_targets (
  deployment_id TEXT NOT NULL,
  robot_id TEXT NOT NULL,
  status TEXT NOT NULL DEFAULT 'pending',
  sync_job_id TEXT NOT NULL DEFAULT ''
);

CREATE TABLE IF NOT EXISTS desired_state (
  robot_id TEXT PRIMARY KEY,
  artifact_id TEXT NOT NULL DEFAULT '',
  version_major INTEGER NOT NULL DEFAULT 0,
  version_minor INTEGER NOT NULL DEFAULT 0,
  version_patch INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS agent_sessions (
  robot_id TEXT PRIMARY KEY,
  connected_at BIGINT NOT NULL DEFAULT 0,
  last_seen_at BIGINT NOT NULL DEFAULT 0
);
