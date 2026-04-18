#!/usr/bin/env bash
# Build and run host unit/integration tests with coverage report.
#
# Environment variables:
#   MIN_LINE_COVERAGE  Minimum line coverage % required to pass (default: 0 = disabled).
#                      Overrides the fail-under-line value in gcovr.cfg.
#                      Example: MIN_LINE_COVERAGE=50 ./Tools/Scripts/build-test.sh
set -euo pipefail

PRESET="Host-Test"
BUILD_DIR="build/$PRESET"
COVERAGE_DIR="${COVERAGE_DIR:-build/coverage-user}"
MIN_LINE_COVERAGE="${MIN_LINE_COVERAGE:-0}"

echo "==> Configuring: $PRESET"
cmake --preset "$PRESET"

echo "==> Building: $PRESET"
cmake --build --preset "$PRESET"

echo "==> Running tests"
cd "$BUILD_DIR"
ctest --output-on-failure
cd - > /dev/null

mkdir -p "$COVERAGE_DIR"

# ---------------------------------------------------------------------------
# Full report — all instrumented source files, no filter.
# Use this to verify coverage instrumentation is working end-to-end.
# ---------------------------------------------------------------------------
echo "==> Generating full instrumentation report -> $COVERAGE_DIR/full/"
mkdir -p "$COVERAGE_DIR/full"
gcovr \
    --root . \
    --fail-under-line 0 \
    --html-details "$COVERAGE_DIR/full/index.html" \
    --print-summary \
    "$BUILD_DIR"

# ---------------------------------------------------------------------------
# Production gate — Tasks/ only.
# This is the number that matters for CI. Raise MIN_LINE_COVERAGE as domain
# logic is extracted into testable units.
# ---------------------------------------------------------------------------
echo "==> Generating production coverage report (Tasks/) -> $COVERAGE_DIR/tasks/"
mkdir -p "$COVERAGE_DIR/tasks"
gcovr \
    --root . \
    --filter Tasks/ \
    --fail-under-line "$MIN_LINE_COVERAGE" \
    --html-details "$COVERAGE_DIR/tasks/index.html" \
    --xml "$COVERAGE_DIR/tasks/coverage.xml" \
    --print-summary \
    "$BUILD_DIR"

echo ""
echo "==> Reports (open in browser):"
echo "     Full (instrumentation check) : $COVERAGE_DIR/full/index.html"
echo "     Production gate (Tasks/)     : $COVERAGE_DIR/tasks/index.html"
echo "     Min line coverage enforced   : ${MIN_LINE_COVERAGE}%"
