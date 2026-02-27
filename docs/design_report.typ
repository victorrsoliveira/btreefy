// BTreeFy Design Report — Typst source
#set document(
  title: "BTreeFy — Blackboard–Runner Notification: Design Report",
  author: "Victor Oliveira",
  date: datetime(year: 2026, month: 2, day: 27),
)

#set page(
  paper: "a4",
  margin: (x: 2cm, y: 2.5cm),
  header: context {
    if counter(page).get().first() > 1 [
      #set text(8pt, fill: luma(120))
      _BTreeFy — Design Report_
      #h(1fr)
      #counter(page).display()
    ]
  },
)

#set text(font: "New Computer Modern", size: 11pt)
#set par(justify: true, leading: 0.7em)
#set heading(numbering: "1.1")
#show heading.where(level: 1): it => {
  v(0.8em)
  text(size: 16pt, weight: "bold", fill: rgb("#1a5276"), it)
  v(0.4em)
  line(length: 100%, stroke: 0.5pt + rgb("#1a5276"))
  v(0.3em)
}
#show heading.where(level: 2): it => {
  v(0.6em)
  text(size: 13pt, weight: "bold", fill: rgb("#2c3e50"), it)
  v(0.2em)
}
#show heading.where(level: 3): it => {
  v(0.4em)
  text(size: 11pt, weight: "bold", fill: rgb("#34495e"), it)
  v(0.1em)
}

// Code block styling
#show raw.where(block: true): it => {
  set text(size: 8.5pt)
  block(
    width: 100%,
    fill: rgb("#f8f9fa"),
    stroke: 0.5pt + rgb("#dee2e6"),
    radius: 4pt,
    inset: 10pt,
    it,
  )
}
#show raw.where(block: false): it => {
  box(
    fill: rgb("#f0f0f0"),
    outset: (x: 2pt, y: 2pt),
    radius: 2pt,
    it,
  )
}

// Info box helper
#let infobox(title, body, color: rgb("#3498db")) = {
  block(
    width: 100%,
    stroke: (left: 3pt + color),
    fill: color.lighten(92%),
    radius: (right: 4pt),
    inset: 12pt,
  )[
    #text(weight: "bold", fill: color, size: 10pt)[#title] \
    #body
  ]
}

// Tip box
#let tipbox(body) = infobox("Tip", body, color: rgb("#27ae60"))

// Warning box
#let warnbox(body) = infobox("Warning", body, color: rgb("#e67e22"))

// ────────────────────────────────────────────────
// Title page
// ────────────────────────────────────────────────

#align(center)[
  #v(3cm)
  #text(size: 28pt, weight: "bold", fill: rgb("#1a5276"))[
    BTreeFy
  ]
  #v(0.3cm)
  #text(size: 16pt, fill: rgb("#2c3e50"))[
    Blackboard–Runner Notification \
    Design Report
  ]
  #v(1.5cm)
  #line(length: 40%, stroke: 1pt + rgb("#1a5276"))
  #v(1cm)
  #text(size: 12pt)[Victor Oliveira]
  #v(0.3cm)
  #text(size: 10pt, fill: luma(100))[February 2026]
  #v(3cm)
]

#pagebreak()

// ────────────────────────────────────────────────
// Table of Contents
// ────────────────────────────────────────────────

#outline(indent: 1.5em, depth: 2)

#pagebreak()

// ────────────────────────────────────────────────
// Content
// ────────────────────────────────────────────────

= Problem Statement

In `btf_blackboard.c`, the function `btf_blackboard_update_data` needs to notify the runner that the blackboard has changed:

```c
btf_runner_notify_event(BTF_RUNNER_BLACKBOARD_EVT);
```

However, `btf_runner_notify_event` requires a `struct btf_runner*` parameter, and the blackboard has no reference to a runner object.

Adding a runner pointer directly to `struct btf_blackboard` would create a *cyclic dependency*:

#align(center)[
  #block(
    inset: 16pt,
    radius: 6pt,
    fill: rgb("#fdf2e9"),
    stroke: 0.5pt + rgb("#e67e22"),
  )[
    #text(size: 10pt)[
      Runner #sym.arrow.r Tree #sym.arrow.r Blackboard #sym.arrow.r #text(fill: red, weight: "bold")[Runner ✗]
    ]
  ]
]

== Current Architecture

The ownership chain is:
- A *runner* holds a pointer to a *tree* (`btf_tree_st`)
- A *tree* holds an array of *nodes* (`btf_node[]`)
- The *blackboard* is a standalone static object, defined via macros (`BTF_BLACKBOARD_DEFINE`), with no upward reference

The blackboard is architecturally *independent* — it does not belong to the tree or runner. This means there is no natural path from blackboard to runner.

Additionally, there is an inconsistency between runner implementations:

#figure(
  table(
    columns: (auto, 1fr, 1fr),
    align: (left, left, left),
    stroke: 0.5pt + luma(180),
    inset: 8pt,
    table.header(
      table.cell(fill: rgb("#2c3e50"))[#text(fill: white, weight: "bold")[]],
      table.cell(fill: rgb("#2c3e50"))[#text(fill: white, weight: "bold")[Zephyr Runner]],
      table.cell(fill: rgb("#2c3e50"))[#text(fill: white, weight: "bold")[POSIX Runner]],
    ),
    [*Notify signature*], [`btf_runner_notify_event(uint32_t evt)`], [`btf_runner_notify_event(struct btf_runner *runner, uint32_t evt)`],
    [*Event mechanism*], [Global `K_EVENT`], [Per-runner `pthread_cond_t`],
    [*Runner ref needed?*], [No (global state)], [Yes],
  ),
  caption: [Runner implementation inconsistency],
)

#pagebreak()

= Approach A — Callback (Runner Passes Blackboard)

The blackboard gets a *generic callback* (`on_update`) — a function pointer and an opaque `void*` context. The runner passes itself as the context when registering. The blackboard has zero knowledge of the runner.

== Dependency Flow

#align(center)[
  #block(inset: 12pt, radius: 6pt, fill: rgb("#eaf2f8"), stroke: 0.5pt + rgb("#3498db"))[
    Runner #sym.arrow.r.long #text(size: 9pt)[(passes blackboard param & registers callback)] #sym.arrow.r.long Blackboard \
    Blackboard #sym.arrow.r.long #text(size: 9pt)[(calls generic `void(*fn)(void*)` )] #sym.arrow.r.long Runner callback
  ]
]

== Key Changes

=== `btf_blackboard.h` — New callback fields

```c
typedef void (*btf_blackboard_notify_fn_t)(void *context);

struct btf_blackboard
{
    void  *data;
    size_t data_size;
    btf_blackboard_notify_fn_t on_update;         // NEW
    void                      *on_update_context;  // NEW
};
```

=== `btf_blackboard.c` — Calls callback (no runner include)

```c
// NOTE: #include "btf_tree_runner.h" is REMOVED

int32_t btf_blackboard_update_data(struct btf_blackboard *blackboard,
                                   size_t offset, void *data, size_t size)
{
    memcpy((uint8_t *) blackboard->data + offset, data, size);
    if (blackboard->on_update != NULL)
        blackboard->on_update(blackboard->on_update_context);
    return 0;
}
```

=== `btf_tree_runner_posix.c` — Bridge callback and registration

```c
static void blackboard_change_callback(void *context)
{
    struct btf_runner *runner = (struct btf_runner *) context;
    btf_runner_notify_event(runner, BTF_RUNNER_BLACKBOARD_EVT);
}

int32_t btf_runner_init(struct btf_runner *runner, btf_tree_st *tree,
                        struct btf_blackboard *blackboard, // NEW param
                        struct btf_runner_config *config)
{
    // ...
    if (blackboard != NULL)
        btf_blackboard_set_notify(blackboard,
                                  blackboard_change_callback, runner);
    // ...
}
```

#tipbox[The `BTF_RUNNER_BLACKBOARD_EVT` constant is only referenced *inside the runner's bridge callback*, never in the blackboard code. This keeps the blackboard fully agnostic.]

#pagebreak()

= Approach B — Event Bus / Mediator

A standalone *event bus module* acts as a central mediator. The blackboard publishes events to the bus; the runner subscribes to the bus. Neither module knows the other exists.

== Dependency Flow

#align(center)[
  #block(inset: 12pt, radius: 6pt, fill: rgb("#eafaf1"), stroke: 0.5pt + rgb("#27ae60"))[
    Blackboard #sym.arrow.r.long Event Bus #sym.arrow.l.long Runner \
    Event Bus #sym.arrow.r.long #text(size: 9pt)[(dispatches to)] #sym.arrow.r.long Runner callback
  ]
]

== Key Changes

=== `btf_event_bus.h` — New module

```c
#define BTF_EVENT_BUS_MAX_SUBSCRIBERS 4

typedef void (*btf_event_bus_handler_t)(uint32_t event, void *context);

void    btf_event_bus_init(void);
int32_t btf_event_bus_subscribe(uint32_t event_mask,
                                btf_event_bus_handler_t handler,
                                void *context);
void    btf_event_bus_publish(uint32_t event);
```

=== `btf_blackboard.c` — Publishes via bus

```c
#include "btf_event_bus.h"  // instead of btf_tree_runner.h

int32_t btf_blackboard_update_data(...)
{
    memcpy(...);
    btf_event_bus_publish(BTF_RUNNER_BLACKBOARD_EVT);
    return 0;
}
```

=== `btf_tree_runner_posix.c` — Subscribes via bus

```c
static void runner_event_bus_handler(uint32_t event, void *context)
{
    struct btf_runner *runner = (struct btf_runner *) context;
    btf_runner_notify_event(runner, event);
}

int32_t btf_runner_init(...)
{
    // ...
    btf_event_bus_subscribe(BTF_RUNNER_BLACKBOARD_EVT,
                            runner_event_bus_handler, runner);
    // ...
}
```

#infobox("Note")[The event bus uses a static array of subscribers with a compile-time maximum (`BTF_EVENT_BUS_MAX_SUBSCRIBERS = 4`), suitable for embedded targets. No dynamic allocation.]

#pagebreak()

= Approach C — Callback (Tree Holds Blackboard)

The *tree* (`btf_tree_st`) holds a `struct btf_blackboard*`. The runner accesses it via `tree->blackboard` during init and registers a callback. The `btf_runner_init` signature stays unchanged.

== Dependency Flow

#align(center)[
  #block(inset: 12pt, radius: 6pt, fill: rgb("#fef9e7"), stroke: 0.5pt + rgb("#f39c12"))[
    Runner #sym.arrow.r.long Tree #sym.arrow.r.long #text(size: 9pt)[(holds NEW)] #sym.arrow.r.long Blackboard \
    Runner #sym.arrow.r.long #text(size: 9pt)[(registers callback via `tree->blackboard`)] #sym.arrow.r.long Blackboard
  ]
]

== Key Changes

=== `btreefy_objs.h` — Tree holds blackboard pointer

```c
struct btf_blackboard;  // forward declaration (no include)

typedef struct
{
    struct btf_node       *nodes;
    uint32_t               size;
    uintptr_t              running_node_index;
    struct btf_blackboard *blackboard;  // NEW
} btf_tree_st;
```

=== `btf_tree_runner_posix.c` — Reads `tree->blackboard`

```c
int32_t btf_runner_init(struct btf_runner *runner, btf_tree_st *tree,
                        struct btf_runner_config *config) // UNCHANGED
{
    // ...
    if (tree->blackboard != NULL)
        btf_blackboard_set_notify(tree->blackboard,
                                  blackboard_change_callback, runner);
    // ...
}
```

=== Application init

```c
btf_init(&tree, nodes, tree_size);
tree.blackboard = &_btf_blackboard_obj_app_blackboard; // manual
btf_runner_init(&runner, &tree, &config);
```

#warnbox[The tree becomes a _carrier_ for the blackboard reference, even though the tree itself never uses it — only the runner does. `tree.blackboard` must be set *before* `btf_runner_init` is called.]

#pagebreak()

= Comparison

== Pros & Cons

#figure(
  table(
    columns: (1.8fr, 1fr, 1fr, 1fr),
    align: (left, center, center, center),
    stroke: 0.5pt + luma(180),
    inset: 7pt,
    table.header(
      table.cell(fill: rgb("#2c3e50"))[#text(fill: white, weight: "bold", size: 9pt)[Criterion]],
      table.cell(fill: rgb("#2c3e50"))[#text(fill: white, weight: "bold", size: 9pt)[A — Callback\ (param)]],
      table.cell(fill: rgb("#2c3e50"))[#text(fill: white, weight: "bold", size: 9pt)[B — Event Bus]],
      table.cell(fill: rgb("#2c3e50"))[#text(fill: white, weight: "bold", size: 9pt)[C — Callback\ (via tree)]],
    ),
    [No cyclic dependency], [✅], [✅], [✅],
    [Blackboard decoupled], [✅], [✅], [✅],
    [No new modules], [✅], [❌], [✅],
    [No `btf_runner_init` change], [❌], [✅], [✅],
    [No change to `btf_tree_st`], [✅], [✅], [❌],
    [No unnecessary stored refs], [✅], [✅], [⚠],
    [Multiple observers], [❌], [✅], [❌],
    [Memory overhead], [\~16 B], [\~64 B], [\~16 B + 1 ptr],
    [Init ordering sensitivity], [Low], [Low], [⚠ High],
  ),
  caption: [Feature comparison of the three approaches],
)

#v(0.5em)

== Software Engineering Principles

#figure(
  table(
    columns: (1.5fr, 1fr, 1fr, 1fr),
    align: (left, center, center, center),
    stroke: 0.5pt + luma(180),
    inset: 7pt,
    table.header(
      table.cell(fill: rgb("#2c3e50"))[#text(fill: white, weight: "bold", size: 9pt)[Principle]],
      table.cell(fill: rgb("#2c3e50"))[#text(fill: white, weight: "bold", size: 9pt)[A]],
      table.cell(fill: rgb("#2c3e50"))[#text(fill: white, weight: "bold", size: 9pt)[B]],
      table.cell(fill: rgb("#2c3e50"))[#text(fill: white, weight: "bold", size: 9pt)[C]],
    ),
    [Single Responsibility], [✅], [✅], [⚠],
    [Dependency Inversion], [✅], [✅], [✅],
    [Low Coupling], [✅], [✅], [⚠],
    [High Cohesion], [✅], [⚠], [⚠],
    [KISS], [✅], [❌], [✅],
    [YAGNI], [✅], [⚠], [✅],
  ),
  caption: [Evaluation against software engineering principles],
)

== Comparison with Industry-ready Behavior Tree Libraries

Research into established, industry-standard Behavior Tree implementations confirms that the *Callback/Observer pattern (Approach A)* aligns perfectly with best practices for building dynamic, reactive behavior trees.

The primary necessity for this pattern is achieving *Reactive Execution*. A naive behavior tree requires the runner to constantly poll (`tick()`) the tree to detect changes in the environment. By allowing the blackboard to notify the runner of events, the runner can sleep (e.g., via `pthread_cond_wait`), saving valuable CPU cycles. This is crucial for resource-constrained embedded systems and is handled similarly in major frameworks:

*Unreal Engine Behavior Trees*: Unreal Engine implements exactly this design via "Blackboard Observers." When the tree executes, decorators subscribe to specific blackboard keys. Upon modification, the blackboard emits a notification, alerting the engine to re-evaluate branches, triggering "Observer Aborts", all without the blackboard needing specific knowledge of the nodes.

*BehaviorTree.CPP*: This popular robotics library fundamentally relies on polling via Port communication during the tick cycle. Consequently, the community frequently discusses the need for event-driven reactive updates like reacting to asynchronous ROS messages. Developers often have to build custom wrappers or decorators to simulate the exact callback mechanism proposed here. Implementing the observer natively inside `btf_blackboard.c` is actually a step ahead of standard `BehaviorTree.CPP` polling mechanisms.

*NodeCanvas and Behavior Designer*: These Unity tools leverage the C-sharp event system under the hood. Their Shared Variables, also known as Blackboards, fire `delegate` callbacks upon modification. The tree engine subscribes to these delegates during initialization—directly mirroring the top-down dependency flow defined in Approach A.

In summary, implementing *Approach A* with a callback registered via dependency injection in `btf_runner_init` is technically consistent with top-tier reactive BT engines like Unreal's, while remaining perfectly suited for the memory and scale constraints of embedded C programming.

#pagebreak()

= Recommendation: Approach A

*Approach A (Callback — runner passes blackboard)* is the best fit for BTreeFy:

+ *Cleanest SRP* — the tree stays purely a tree; the blackboard is purely a data store with a notification hook; the runner is the only module aware of both.

+ *Explicit dependency* — the blackboard parameter in `btf_runner_init` makes the dependency _visible at the API level_ rather than hidden inside a struct field or a global module.

+ *KISS + YAGNI* — no new modules, no conceptual stretching of the tree's purpose, no global state. It solves exactly the problem with minimal additions.

+ *The signature change is a feature* — it documents the runner's dependency on the blackboard for anyone reading the API.

#tipbox[Do not store `runner->blackboard`. Use the parameter locally in `btf_runner_init` to call `btf_blackboard_set_notify`, then discard it. The runner struct stays unchanged.]

== Final Shape

```c
int32_t btf_runner_init(struct btf_runner *runner,
                        btf_tree_st *tree,
                        struct btf_blackboard *blackboard,
                        struct btf_runner_config *config);

// Inside btf_runner_init:
if (blackboard != NULL)
{
    btf_blackboard_set_notify(blackboard,
                              blackboard_change_callback,
                              runner);
}
// No runner->blackboard = blackboard; (not needed)
```
