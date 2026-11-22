; Jinja's tags and expressions. The SQL between them is highlighted by the
; SQL grammar injections.scm hands it to.

(comment) @comment @spell

[
  "{{" "{{-" "{{+" "}}" "-}}" "+}}"
  "{%" "{%-" "{%+" "%}" "-%}" "+%}"
] @tag.delimiter

[
  "if" "elif" "else" "endif"
] @keyword.conditional

[
  "for" "endfor" "in" "recursive" "break" "continue"
] @keyword.repeat

[
  "macro" "endmacro" "call" "endcall" "test" "endtest"
  "materialization" "endmaterialization"
] @keyword.function

[
  "include" "import" "from" "as" "ignore" "missing" "without" "context"
] @keyword.import

[
  "set" "endset" "filter" "endfilter" "raw" "endraw" "with" "endwith" "do"
  "snapshot" "endsnapshot" "docs" "enddocs"
] @keyword

[
  "and" "or" "not" "is" "not in"
] @keyword.operator

[
  "==" "!=" "<" "<=" ">" ">=" "+" "-" "*" "/" "//" "%" "**" "~" "=" "|"
] @operator

[ "(" ")" "[" "]" "{" "}" ] @punctuation.bracket
[ "," "." ":" ] @punctuation.delimiter

(string) @string
(escape_sequence) @string.escape
(integer) @number
(float) @number.float
(boolean) @boolean
(none) @constant.builtin

(identifier) @variable

(keyword_argument name: (identifier) @variable.parameter)
(default_parameter name: (identifier) @variable.parameter)
(parameters (identifier) @variable.parameter)

(attribute attribute: (identifier) @variable.member)

(call function: (identifier) @function.call)
(call function: (attribute attribute: (identifier) @function.method.call))

(filter_expression filter: (identifier) @function.call)
(filter_expression filter: (dotted_name (identifier) @function.call))
(filter_chain filter: (identifier) @function.call)
(test_expression test: (identifier) @function.call)

(macro_definition name: (identifier) @function)
(test_definition name: (identifier) @function)
(materialization_definition name: (identifier) @function)
(snapshot_block name: (identifier) @label)
(docs_block name: (identifier) @label)

; dbt's context: what every model and macro can call or read
((call function: (identifier) @function.builtin)
  (#any-of? @function.builtin
    "ref" "source" "config" "var" "env_var" "is_incremental" "log" "return"
    "run_query" "statement" "load_result" "doc" "metric" "fromjson" "tojson"
    "fromyaml" "toyaml" "set" "zip" "dispatch" "print" "load_relation"
    "should_full_refresh" "should_store_failures"))

((identifier) @variable.builtin
  (#any-of? @variable.builtin
    "this" "target" "model" "adapter" "api" "builtins" "context" "dbt"
    "dbt_version" "env" "exceptions" "execute" "flags" "graph" "invocation_id"
    "modules" "project_name" "run_started_at" "schemas" "selected_resources"
    "sql" "loop" "caller" "varargs" "kwargs" "validation" "database" "schema"))

(raw_content) @markup.raw
