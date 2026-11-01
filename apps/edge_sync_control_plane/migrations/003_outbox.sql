-- Durable command dispatch: the transactional outbox. A sync job and its
-- dispatch intent are written atomically; the dispatcher delivers and marks
-- delivery/execution states separately.
ALTER TABLE sync_jobs ADD COLUMN IF NOT EXISTS command_id TEXT NOT NULL DEFAULT '';
ALTER TABLE sync_jobs ADD COLUMN IF NOT EXISTS delivery_state TEXT NOT NULL DEFAULT 'pending';
ALTER TABLE sync_jobs ADD COLUMN IF NOT EXISTS attempts INTEGER NOT NULL DEFAULT 0;
ALTER TABLE sync_jobs ADD COLUMN IF NOT EXISTS updated_at BIGINT NOT NULL DEFAULT 0;

CREATE TABLE IF NOT EXISTS outbox (
  id BIGSERIAL PRIMARY KEY,
  command_id TEXT UNIQUE NOT NULL,
  target_robot TEXT NOT NULL,
  source_robot TEXT NOT NULL DEFAULT '',
  artifact_id TEXT NOT NULL DEFAULT '',
  delivery_state TEXT NOT NULL DEFAULT 'pending',
  attempts INTEGER NOT NULL DEFAULT 0,
  created_at BIGINT NOT NULL DEFAULT 0
);
