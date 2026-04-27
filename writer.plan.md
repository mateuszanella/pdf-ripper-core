## Plan: Writer Save Pipeline v1

Build a first save path now, but keep scope intentionally narrow: full rewrite only, with no incremental update in v1. The implementation adds a writer abstraction parallel to reader, a serializer boundary that owns object model to byte emission, and a document save API that orchestrates canonical PDF output order (header, body objects, xref, trailer, EOF). This keeps the architecture aligned with your dynamic library goal (read-only, write-only, read-write, and in-memory flows) while avoiding premature complexity. Unknown/specialized objects are handled through generic PDF primitive serialization with opaque pass-through rules, and existing signatures are treated as non-preservable under full rewrite (documented behavior) until v2 incremental save.

**Steps**
<!-- 1. Add writer abstraction interfaces in include/core/writer for byte-oriented output only, mirroring reader responsibilities and keeping storage concerns isolated. -->
<!-- 2. Implement file-backed writer in include/core/writer and src/core/writer with open, write, seek, tell, flush, and close behavior plus structured error propagation. -->
3. Add serializer orchestration in include/core/serializer and src/core/serializer to emit canonical PDF sections in order and to centralize offset/xref bookkeeping.
4. Introduce document-owned change tracking in include/core/document/change_set.hpp and wire it into document and object mutator paths using indirect references and states (new, modified, deleted).
5. Add centralized object allocation in document using max_object_number + 1 with generation 0 to avoid collisions for newly created objects.
6. Add save API to document in include/core/document.hpp and implement in src/core/document.cpp as v1 full rewrite strategy using writer + serializer.
7. Define unknown-object handling policy in serializer: unchanged unknown objects round-trip as opaque/generic primitives; unsupported semantic mutations fail with explicit unsupported-mutation errors.
8. Document signature behavior for v1 full rewrite: existing signatures are expected to become invalid unless re-signed; defer preservation to v2 incremental mode.
9. Add focused tests for writer and serializer units, then integration roundtrip tests (read -> modify -> save -> read) with representative fixture PDFs.
10. Update build targets in CMake to expose library-oriented writer/serializer/document save components and optional test/example toggles.

**Verification**
- Build the project and run unit/integration targets after each phase to isolate regressions early.
- Validate saved PDFs with at least two external readers to confirm structural compatibility.
- Add roundtrip assertions for object reachability, xref consistency, trailer correctness, and EOF markers.
- Include malformed/edge-case corpus checks to confirm deterministic errors and no silent corruption.
- Confirm documented v1 behavior for signed PDFs through explicit regression cases.

**Decisions**
- Decision: implement writing now with constrained v1 scope to unblock architecture and API modeling.
- Decision: full rewrite first, incremental update deferred to v2.
- Decision: prioritize generic PDF primitive serialization over exhaustive typed object support.
- Decision: keep change state document-owned, not object-owned, to avoid recursive dirty checks and accidental write amplification.
- Decision: treat digital signatures as opaque and non-preservable in v1 full rewrite; handle preservation only in v2 append-only incremental save.
