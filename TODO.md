# TODO

## 1) Parser and Architecture Refactor

- [ X ] Split `parser` into smaller components
  - Separate parser implementations from object-building logic, the parser class should
        just get the correct parser implementation and build the objects through a builder class

---

## 2) Errors and Diagnostics

- [ X ] Refactor error model
  - Replace integer-like errors with lightweight structured error objects
  - Preserve non-exception workflow (`std::expected` style)
  - Include richer context (location, object/ref, parser stage)

---

## 3) Writing and Saving (major milestone)

- [ X ] Implement output pipeline (`writer` + serializer layer)
  - Add a `writer` abstraction mirroring `reader` responsibilities (byte-oriented I/O only)
  - Implement file-backed writer
  - Define serializer orchestration boundary (object model -> PDF bytes)

- [ ] Implement document save flow (v1: full rewrite)
  - Add `document::save(...)` API using full rewrite as the first strategy
  - Serialize header, body objects, xref table, trailer, and EOF markers in canonical order
  - Ensure resulting files are readable by common PDF readers
  - Keep incremental update support out of v1 (explicitly deferred)

- [ ] Add change tracking infrastructure (document-owned, not object-owned flags)
  - Introduce `change_set` class owned by `document`
  - Track object state by indirect reference (`new`, `modified`, `deleted`)
  - Register mutations through object mutator methods via `indirect_object` owner/document context
  - Avoid "accessed == dirty" and recursive `is_dirty()` traversal

- [ ] Add object allocation policy for newly created objects
  -  Allocate object numbers as `max_object_number + 1` (generation `0`) for v1
  -  Centralize allocation in `document` to avoid collisions

- [ ] Plan v2 save strategy (incremental update)
  - Add append-only incremental write mode
  - Handle `/Prev` chaining and xref/trailer history correctly
  - Handle generation/free-entry rules for deleted/reused objects

---

## 4) Testing and Validation

- [ ] Add unit tests
- [ ] Add workflow/integration tests
- [ ] Add roundtrip tests for writing (`read -> modify -> save -> read`)
- [ ] Add regression corpus for malformed and edge-case PDFs

---

## 5) Build and Packaging

> Sort of done, but not checking this until I test library consumption from an external project

- [ ] Add proper CMake targets for library consumption
  - Build as a reusable library (not only `main()` testbed)
  - Export/install targets
  - Add option toggles for examples/tests

---

## 6) Documentation and Developer Experience

- [ ] Add API documentation
- [ ] Add usage examples (read-only and read/modify/save flows)
- [ ] Document writer/save behavior and guarantees (v1 vs v2)
- [ ] Document internal architecture (`reader/parser`, `writer/serializer`, `change_set`)

---

## 7) Feature Growth

- [ ] Add additional PDF features incrementally as needed
- [ ] Prioritize features based on test coverage and real-world documents

---

## 8) Reader/Writer error propagation and diagnostics

- [ ] Add some type of error propagation for operations that may fail when reading/writing.
