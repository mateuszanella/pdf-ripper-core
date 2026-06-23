# TODO

## Priority 1 — Serializer Tests

- [] **Add serializer tests** — The serialization pipeline is the most critical write path.
  No tests exist for any serializer component. Protect against regressions in:
  - `default_header_serializer` — version string, comment, whitespace
  - `default_object_serializer` — all 10 object types, edge cases (deep nesting,
    empty containers, large streams, special characters in strings/names)
  - `default_indirect_object_serializer` — obj/endobj wrapping, stream dict + data
  - `default_cross_reference_table_serializer` — contiguous, split, and empty
    subsections; 20-byte entry formatting
  - `default_trailer_serializer` — `trailer <<...>> startxref %%EOF` block,
    `/Prev`, `/ID`, `xref_offset` correctness
  - Round-trip: serialize → re-parse → compare

## Priority 2 — Save Strategies

- [] **Add raw save strategy** — Pass-through save that copies unchanged objects
  byte-for-byte from the input file, only serializing modified objects. Produces
  a file where unmodified entries retain their original formatting. Requires:
  - Modify tracking (dirty flag on `cross_reference_entry` or set of modified
    object numbers)
  - Memory-mapped or buffered read of original raw bytes for unchanged objects
  - A `save_mode` enum or separate `document::save_raw()` method

- [] **Add incremental update save strategy** — Append a new cross-reference
  section and trailer to the existing file without rewriting earlier content.
  This is the standard PDF editing approach (used by Acrobat, etc.). Requires:
  - Modify tracking (shared with raw save)
  - Write only new/modified objects, then new `xref` section + `trailer`
  - Update `/Prev` chain in the new trailer to point to previous `startxref`
  - Handle object generation incrementing for reused objects
  - Proper free list management (`mark_deleted` → free entry in new section)

- [] **Add optimized rewrite save strategy** (lower priority) — Full rewrite that
  goes beyond the current `save()` squash behavior:
  - Remove free/deleted entries (already done by squash)
  - Compact cross-reference subsections
  - Deduplicate identical objects
  - Normalize numeric representation (`1.0` → `1`)

## Priority 3 — Stream Abstractions

- [] **Add proper stream abstractions** — Move beyond the bare `vector<byte>`
  payload. A filter-chain abstraction that:
  - Reads `/Filter` entries from the stream dictionary
  - Automatically decodes on parse and re-encodes on serialize
  - Supports common filters: `/FlateDecode`, `/ASCIIHexDecode`,
    `/ASCII85Decode`, `/LZWDecode` (at minimum Flate, the rest can be stubs)
  - Tracks which filters have been applied vs. which are declared
  - Handles `/DecodeParms` parameter propagation
  - Integrates with the object parser so decoded content is available on access

## Priority 4 — Convenience Wrappers

Higher-level typed abstractions (following the `catalog`/`pages`/`page` pattern):

### High value

- [] **`document_info`** — Typed wrapper over the `/Info` dictionary. Provides
  get/set for: `Title`, `Author`, `Subject`, `Keywords`, `Creator`, `Producer`,
  `CreationDate`, `ModDate`. Exposition: `doc.info().set_title("...")`,
  `doc.info().author()`.

- [] **`metadata`** — Wrap `/Metadata` XMP stream. Read and write XMP packet
  as a stream. Integrates with `document_info` for auto-sync.

- [] **`page_resources`** — Typed view over a page's `/Resources` dictionary.
  Access to `Font`, `XObject`, `ExtGState`, `ColorSpace`, `Pattern`,
  `Shading`, `Properties` sub-dictionaries.

- [] **`page_content`** — Wrapper around a page's content stream(s). Expose
  the raw content operators for inspection and append new operators (text,
  graphics, images). Foundation for any content generation.

### Medium value

- [] **`outline`** / `outline_item` — Typed view over `/Outlines` dictionary
  and individual `/Outline` items. Read/write bookmark hierarchy.

- [] **`annotation`** — Typed wrapper for annotation dictionaries. Common
  subtypes: text, link, highlight, stamp, ink.

- [] **`xref_stream`** — Parse and write cross-reference streams (PDF 1.5+
  `/Type /XRef`). Needed for modern PDF compatibility.

- [] **`name_tree`** / `number_tree` — Generic wrappers for name tree and
  number tree lookups (used by outlines, embedded files, AcroForm).

### Lower value (requires content-level work)

- [] **`font`** — Typed wrapper for font dictionaries. `/Type /Font` with
  subtypes: Type0, TrueType, Type1, CIDFontType2. Font descriptor.

- [] **`xobject`** — Wrapper for external objects: images (`/Subtype /Image`),
  form XObjects (`/Subtype /Form`). Image data access, dimensions, color space.

## Infrastructure

- [] **Modify tracking** — Shared prerequisite for raw and incremental save
  strategies. Two approaches to consider:
  - Dirty flag per `cross_reference_entry` (set by `dictionary::set()` etc.)
  - Track set of modified object numbers at the `document` level
  - Second approach is simpler but needs care with deep mutations

- [] **Object number reuse** — Track free object numbers and reuse them rather
  than always incrementing. Important for incremental updates to not exhaust
  the object number space.

- [] **PDF string encoding** — `object.hpp` `@TODO`: promote `std::string` to a
  proper class that differentiates UTF-8, Latin-1, PDFDocEncoding, and hex vs.
  literal representation (`(text)` vs. `<hex>`).

- [] **`page::prune_page()`** — Implement recursive deletion of a page and all
  its child objects from the cross-reference table. Currently throws
  `logic_exception`. Blocked on better common-object abstractions (resources,
  content streams, annotations).
