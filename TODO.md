# TODO

## Errors and Diagnostics

- [ ] Refactor error model again.
    > To be honest, using errors doesnt really seem like a smart option looking from now.
    > Most errors are unrecoverable and should just be fatal, and the ones that are recoverable 
    > are mostly 'element not found' type errors that can be handled with `std::optional` or similar.
    > Just using exceptions for unrecoverable errors and `std::optional` for recoverable ones seems 
    > like a much more straightforward approach. This is not even taking into account the fact that using
    > `std::expected` is just really verbose and adds a lot of boilerplate for error handling that is not
    > really necessary in this context.
  - Remove `std::expected` and `noexcept` from all APIs and replace with exceptions for unrecoverable 
    errors and `std::optional` for recoverable ones.

---

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

---

## Reader/Writer error propagation and diagnostics

- [ ] Add some type of error propagation for operations that may fail when reading/writing.
