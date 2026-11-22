# tree-sitter-dbt

A [tree-sitter](https://tree-sitter.github.io) grammar for [dbt](https://www.getdbt.com)
models, macros, tests, snapshots and materializations: Jinja over SQL.

It parses the Jinja in full and leaves the SQL between the tags as `content`
nodes. `queries/injections.scm` hands all of a file's `content` to a SQL grammar
as **one combined document**, so the SQL grammar sees the query whole rather than
a fragment per gap between tags. It does not depend on any dbt tool or language
server.

```sql
{{ config(materialized='incremental') }}
select * from {{ ref('events') }}
{% if is_incremental() %}
where ts > (select max(ts) from {{ this }})
{% endif %}
```

```
(source_file
  (output (call function: (identifier) arguments: (argument_list (keyword_argument ...))))
  (content)
  (output (call function: (identifier) arguments: (argument_list (string ...))))
  (content)
  (if_statement
    condition: (call function: (identifier) arguments: (argument_list))
    consequence: (content)
    consequence: (output (identifier))
    consequence: (content))
  (content))
```

## What it parses

- `{{ … }}`, `{% … %}` and `{# … #}`, with whitespace control (`{{-`, `-%}`, `+%}`).
- Jinja's statements: `if`/`elif`/`else`, `for` (tuple targets, a trailing `if`,
  `recursive`, `else`, `break`, `continue`), `set` and `set` blocks, `macro`,
  `call` blocks, `filter` blocks, `raw`, `with`, `do`, `include`, `import`, `from … import`.
- dbt's blocks: `materialization`, `test`, `snapshot` and `docs`. A docs block's
  text is `markdown_content` and injected as Markdown.
- Jinja's expressions at Jinja's precedence: conditionals, `and`/`or`/`not`,
  comparisons and `in`/`not in`, arithmetic, `~`, filters and tests, attributes,
  subscripts and slices, calls with keyword arguments and `*`/`**`, lists,
  tuples, dicts, adjacent strings (`"a" "b"`) and both spellings of the
  constants (`true`/`True`, `none`/`None`).
- What dbt's own code does that Jinja allows: a `:` after a tag's condition
  (`{% if execute: %}`) and a bare tuple in a subscript (`x['a', 'b']`).

`script/parse-examples` parses every `.sql` file of six of dbt-labs' repositories
at pinned commits (`script/examples.txt`: dbt-utils, dbt-adapters with its global
project, jaffle-shop, dbt-codegen, dbt-audit-helper, dbt-project-evaluator), 770
files, and CI fails on a single error.

## Using it

**Neovim** (nvim-treesitter):

```lua
local parsers = require("nvim-treesitter.parsers").get_parser_configs()
parsers.dbt = {
  install_info = {
    url = "https://github.com/sirh3e/tree-sitter-dbt",
    files = { "src/parser.c", "src/scanner.c" },
    branch = "main",
  },
}
vim.treesitter.language.register("dbt", "dbt")
-- dbt models are .sql files: pick the filetype for your project's paths
vim.filetype.add({ pattern = { [".*/models/.*%.sql"] = "dbt", [".*/macros/.*%.sql"] = "dbt" } })
```

Copy `queries/*.scm` into `queries/dbt/` on your runtime path. SQL inside needs a
`sql` parser (`:TSInstall sql`).

**Rust**:

```toml
[dependencies]
tree-sitter = "0.26"
tree-sitter-dbt = { git = "https://github.com/sirh3e/tree-sitter-dbt" }
```

```rust
let mut parser = tree_sitter::Parser::new();
parser.set_language(&tree_sitter_dbt::LANGUAGE.into())?;
let tree = parser.parse(source, None).unwrap();
```

Node (`tree-sitter-dbt`), Python (`tree_sitter_dbt`) and C bindings are in
`bindings/`.

## Files

The grammar claims the extension `.dbt` for the tree-sitter CLI only; dbt's own
files are `.sql`, which an editor should map to `dbt` by path, as above, rather
than taking `.sql` from the SQL grammar.

## Developing

```sh
npm ci
npx tree-sitter generate     # after changing grammar.js
npx tree-sitter test         # corpus (test/corpus) and highlight tests
./script/parse-examples      # real dbt code, needs the CLI on PATH
cargo test                   # the Rust binding
```

`src/parser.c` is generated and committed; CI fails while it differs from
`grammar.js`. The external scanner (`src/scanner.c`) lexes the two tokens a
regular expression cannot: the text between tags, and everything inside
`{% raw %}`.

## Licence

MIT.
