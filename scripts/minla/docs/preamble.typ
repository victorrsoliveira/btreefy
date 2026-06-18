// ─────────────────────────────────────────────────────────────────────────────
// preamble.typ — Shared Typst Theme for BT MinLA Phase Reports
// Matches the visual identity of docs/presentation.typ in the repo root.
//
// Usage in each phase report:
//   #import "preamble.typ": *
//   #show: report-setup   ← applies page/text rules
// ─────────────────────────────────────────────────────────────────────────────

// ── Colour palette (same as presentation.typ) ─────────────────────────────
#let bg       = rgb("#0d1117")
#let surface  = rgb("#161b22")
#let border   = rgb("#30363d")
#let accent   = rgb("#58a6ff")
#let accent2  = rgb("#3fb950")
#let warn     = rgb("#d29922")
#let clr_text = rgb("#e6edf3")
#let muted    = rgb("#8b949e")

// ── Page & typography setup (applied via show rule) ───────────────────────
#let report-setup(doc) = {
  set page(
    paper: "a4",
    fill: bg,
    margin: (x: 2.4cm, y: 2.2cm),
    numbering: "1",
  )
  set text(font: "Libertinus Serif", fill: clr_text, size: 11pt)
  set heading(numbering: "1.1")
  show heading: set text(fill: accent)
  show strong:  set text(fill: accent)
  show emph:    set text(fill: accent2, style: "italic")
  show link:    set text(fill: accent)
  show raw:     set text(font: "Fira Code", size: 10pt, fill: accent2)
  doc
}

// ── Divider ───────────────────────────────────────────────────────────────
#let divider = line(length: 100%, stroke: 0.6pt + border)

// ── Pill badge ────────────────────────────────────────────────────────────
#let pill(content, color: accent) = box(
  fill: color.transparentize(80%),
  stroke: 0.6pt + color,
  radius: 4pt,
  inset: (x: 6pt, y: 3pt),
  text(fill: color, size: 10pt, content)
)

// ── Inline badge ──────────────────────────────────────────────────────────
#let badge(t, color: accent) = box(
  fill: color,
  radius: 3pt,
  inset: (x: 5pt, y: 2pt),
  text(fill: bg, size: 10pt, weight: "bold", t)
)

// ── Card block ────────────────────────────────────────────────────────────
#let card(content, title: none, color: accent) = block(
  fill: surface,
  stroke: 0.8pt + color.transparentize(60%),
  radius: 6pt,
  inset: 14pt,
  width: 100%,
  {
    if title != none {
      text(fill: color, size: 11pt, weight: "bold", title)
      v(6pt)
    }
    content
  }
)

// ── Algorithm / pseudocode block ──────────────────────────────────────────
#let algo(content, title: none) = card(
  content,
  title: title,
  color: accent2,
)

// ── Result / highlight box ────────────────────────────────────────────────
#let result-box(content) = card(content, color: accent)

// ── Warning / note box ────────────────────────────────────────────────────
#let note-box(content) = card(content, color: warn)

// ── Bullet helper ─────────────────────────────────────────────────────────
#let bullet(content) = pad(left: 8pt, {
  grid(columns: (12pt, 1fr), gutter: 6pt,
    text(fill: accent, "›"), content)
})

// ── Inline citation ───────────────────────────────────────────────────────
#let ref-cite(t) = text(fill: muted, style: "italic", size: 9pt, "[" + t + "]")
