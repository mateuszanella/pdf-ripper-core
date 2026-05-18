# TODO

## Writing and Saving (major milestone)

- [x] Implement output pipeline (`writer` + serializer layer)
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

## Move Reader / Writer to a separate library

> If I plan on actually create modular libraries with similar interfaces, having the reader/writer in
> the same library would just be a lot of repeated code. Having them in separate libraries makes a lot
> of sense, since they are a simple generic abstraction that should remain stable and reusable.

- [ ] Create a new library target (e.g. `ripper_pdf_io`) for reader/writer and related utilities

## Testing and Validation

- [ ] Add unit tests
- [ ] Add workflow/integration tests
- [ ] Add roundtrip tests for writing (`read -> modify -> save -> read`)
- [ ] Add regression corpus for malformed and edge-case PDFs

---

## Build and Packaging

> Sort of done, but not checking this until I test library consumption from an external project

- [ ] Add proper CMake targets for library consumption
  - Build as a reusable library (not only `main()` testbed)
  - Export/install targets
  - Add option toggles for examples/tests

---

## Documentation and Developer Experience

- [ ] Add API documentation
- [ ] Add usage examples (read-only and read/modify/save flows)
- [ ] Document writer/save behavior and guarantees (v1 vs v2)
- [ ] Document internal architecture (`reader/parser`, `writer/serializer`, `change_set`)

---

## Feature Growth

- [ ] Add additional PDF features incrementally as needed
- [ ] Prioritize features based on test coverage and real-world documents
