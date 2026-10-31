# Tests

- Unit tests live in each package's `test/` directory (GoogleTest).
- `support/` holds shared test doubles (e.g. `capturing_logger.hpp`).
- Integration / end-to-end / fault-injection / performance suites land here as
  the sync engine grows (M3+).

Run: `colcon test && colcon test-result --verbose`
