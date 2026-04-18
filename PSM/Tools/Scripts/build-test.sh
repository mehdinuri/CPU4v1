#!/usr/bin/env bash
# Build and run host unit/integration tests with coverage report.
set -euo pipefail

PRESET="Host-Test"
BUILD_DIR="build/$PRESET"
COVERAGE_DIR="build/coverage-report"

echo "==> Configuring: $PRESET"
cmake --preset "$PRESET"

echo "==> Building: $PRESET"
cmake --build --preset "$PRESET"

echo "==> Running tests"
cd "$BUILD_DIR"
ctest --output-on-failure

cd - > /dev/null

echo "==> Generating coverage report -> $COVERAGE_DIR/"
mkdir -p "$COVERAGE_DIR"
gcovr \
    --root . \
    --filter App/Domain/ \
    --html-details "$COVERAGE_DIR/index.html" \
    --xml "$COVERAGE_DIR/coverage.xml" \
    --print-summary \
    "$BUILD_DIR"

echo "==> Coverage report: $COVERAGE_DIR/index.html"
