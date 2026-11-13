# Source navigation

The existing component layout already separates `include/listen_clouds/` public
contracts, `src/` implementation, and `tests/` host protocol/frame tests.
`CMakeLists.txt` and `idf_component.yml` are component entry points. Build and
managed dependency directories are generated artifacts, not source modules.

2026-11-13 review: retained this layout rather than introduce unnecessary path
changes. Protocol and frame host tests pass via the integration host suite.
