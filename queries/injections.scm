; The text between the tags is one SQL document, so a SQL grammar sees the
; query whole rather than a fragment per gap between tags.
((content) @injection.content
  (#set! injection.language "sql")
  (#set! injection.combined))

; A docs block holds Markdown.
((markdown_content) @injection.content
  (#set! injection.language "markdown")
  (#set! injection.combined))

((comment) @injection.content
  (#set! injection.language "comment"))
