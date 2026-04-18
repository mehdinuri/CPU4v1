#!/usr/bin/env bash
# Build and run host unit/integration tests with coverage report.
#
# Environment variables:
#   MIN_LINE_COVERAGE  Overrides gcovr.cfg's fail-under-line for the
#                      production gate. Leave unset to use gcovr.cfg default.
#                      Example: MIN_LINE_COVERAGE=50 ./Tools/Scripts/build-test.sh
set -euo pipefail

PRESET="Host-Test"
BUILD_DIR="build/$PRESET"
COVERAGE_DIR="${COVERAGE_DIR:-build/coverage-user}"

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
# Production gate — scope + threshold come from gcovr.cfg (App/Domain/, 80%).
# Pass MIN_LINE_COVERAGE to override the gcovr.cfg threshold from CI.
# ---------------------------------------------------------------------------
echo "==> Generating production coverage report -> $COVERAGE_DIR/domain/"
mkdir -p "$COVERAGE_DIR/domain"
GCOVR_GATE_ARGS=(--root . --html-details "$COVERAGE_DIR/domain/index.html" --xml "$COVERAGE_DIR/domain/coverage.xml" --print-summary)
if [[ -n "${MIN_LINE_COVERAGE:-}" ]]; then
    GCOVR_GATE_ARGS+=(--fail-under-line "$MIN_LINE_COVERAGE")
fi
gcovr "${GCOVR_GATE_ARGS[@]}" "$BUILD_DIR"

echo ""
echo "==> Reports (open in browser):"
echo "     Full (instrumentation check) : $COVERAGE_DIR/full/index.html"
echo "     Production gate (App/Domain/) : $COVERAGE_DIR/domain/index.html"
