# C++ Module 06 — C++ Casts: Concept Guide

A companion to walk you through the *ideas* behind Module 06. This is **not** a solution or a to-do list. It explains the concepts, the vocabulary, and the mental models you need so that when you write the code, you understand *why* each line is there. Keep it open while you work and come back to sections as they become relevant.

---

## 0. What this module is really about

Every exercise in this module is a disguised question:

> *You have a value of one type. You want to treat it as another type. What exactly happens, and which tool should you use to make it happen safely (or deliberately unsafely)?*

That act — taking a value/object/pointer of type `A` and producing a view of it as type `B` — is called a **conversion**, and when you ask for it explicitly, a **cast**.

The whole module exists to teach you that **"a cast" is not one thing.** C++ gives you four named cast operators, each with a different meaning, different guarantees, and different danger level. The mandatory rule of the module says it out loud: *for each exercise, type conversion must be handled using a specific type of casting.*

So the real learning objective is: **for each situation, know which of the four casts is the correct, intentful one — and be able to defend that choice.**

The three exercises map cleanly onto three of the casts:

| Exercise | Theme | The "right" cast it teaches |
|---|---|---|
| ex00 ScalarConverter | Converting between number/char types | `static_cast` |
| ex01 Serializer | Pointer ↔ integer round-trip | `reinterpret_cast` |
| ex02 Identify real type | "What concrete class is this really?" | `dynamic_cast` |

(The fourth cast, `const_cast`, isn't the star of any exercise, but you must understand it to understand the set.)

---

## 1. Foundations: conversion vs. cast (read this first)

Because you're newer to casts, let's build the vocabulary carefully. Everything later depends on these distinctions.

### 1.1 Implicit conversion (the compiler does it silently)

C++ will convert types *for you* in many situations, with no syntax on your part:

```cpp
int   i = 'A';      // char -> int, silent
double d = i;       // int -> double, silent
if (ptr) { ... }    // pointer -> bool, silent
```

These are **implicit conversions** (a.k.a. *standard conversions* for built-in types, or *user-defined conversions* when a constructor/operator is involved). They feel free, but they can lose information (`double` → `int` truncates) or change meaning, and the compiler usually won't warn loudly. A big part of being a competent C++ programmer is *noticing* where implicit conversions happen.

### 1.2 Explicit conversion = a cast (you ask for it)

When you write the conversion yourself, you're casting. There are two families.

**C-style cast** — the one you know from C:

```cpp
int i = (int)myDouble;
char c = (char)myInt;
```

It's compact, but it's a blunt instrument: a single syntax that the compiler quietly resolves into *whichever* conversion makes it compile — possibly a dangerous one. You can't tell from reading `(T)x` whether it's a harmless numeric conversion or a reinterpret-the-bits reinterpretation. That ambiguity is exactly what C++ wants you to leave behind in this module.

**C++ named casts** — the four operators, each with a narrow, documented job:

```cpp
static_cast<T>(x)
dynamic_cast<T>(x)
reinterpret_cast<T>(x)
const_cast<T>(x)
```

The syntax is verbose *on purpose*. It states your **intent** ("I mean a value conversion" vs. "I mean a bit reinterpretation"), lets the **compiler reject** casts that don't match that intent (a `static_cast` won't let you do something only `reinterpret_cast` is allowed to do), and is **greppable** — you can search a codebase for every `reinterpret_cast` (the scary ones) instantly. You can't grep for `(int)`.

> **Mental model:** A C-style cast says "make it compile somehow." A named cast says "do *exactly this kind* of conversion, and fail if that's not legal."

### 1.3 The big idea behind the four

Sort the casts by **how much they trust the compiler vs. how much they trust you**:

```
SAFE / checked at compile time      <-------->     UNSAFE / "trust me"
static_cast        dynamic_cast        const_cast        reinterpret_cast
(sensible value    (runtime-checked    (only flips        (reinterpret raw
 conversions)       downcasts)          const/volatile)    bits, no checks)
```

- `static_cast` — "this conversion makes logical sense; do it."
- `dynamic_cast` — "is this polymorphic object *actually* that derived type? check at **runtime**."
- `const_cast` — "change only the `const`/`volatile` qualifier, nothing else."
- `reinterpret_cast` — "treat these bits as a different type; I take full responsibility."

Keep this spectrum in your head — it's the single most useful takeaway of the module.

---

## 2. The four casts in depth

### 2.1 `static_cast<T>(expr)` — sensible, compile-time-known conversions

**What it's for:** conversions that are *meaningful* and whose validity can be established at compile time. The compiler knows a real relationship exists between the types.

Typical legitimate uses:
- Numeric conversions between arithmetic types: `int ↔ double ↔ float ↔ char`.
- `void*` → typed pointer (the reverse of an implicit conversion).
- Explicitly invoking a user-defined conversion or a non-explicit constructor.
- **Upcasts** in a hierarchy (derived → base) — though those are usually implicit anyway.
- **Downcasts** (base → derived) *without* a runtime check — only do this when you already know the dynamic type; otherwise it's unsafe.

```cpp
double d = 4.2;
int    i = static_cast<int>(d);     // 4  (fractional part dropped — your intent)
char   c = static_cast<char>(65);   // 'A'
```

**Why it matters here (ex00):** Converting between `char`, `int`, `float`, and `double` is *precisely* what `static_cast` exists for. When the subject says "convert it **explicitly** to the three other data types," it is steering you toward `static_cast` as the intentful tool. Notice that many of these conversions would happen implicitly too — using `static_cast` documents that the narrowing/precision change is *deliberate*, not an accident.

**What `static_cast` does NOT do:**
- It won't reinterpret unrelated pointer types (e.g. `int*` → `float*`). That's `reinterpret_cast` territory.
- It performs **no runtime check** on downcasts. If you `static_cast` a `Base*` to a `Derived*` and the object isn't really a `Derived`, you get undefined behavior. (`dynamic_cast` is the checked alternative.)
- It can't cast away `const`.

### 2.2 `dynamic_cast<T>(expr)` — runtime-checked navigation of a class hierarchy

**What it's for:** safely asking, at **runtime**, "is this base-class pointer/reference *actually* pointing at this particular derived type?" This is the only cast that can *fail gracefully*, because it actually inspects the object while the program runs.

**Hard prerequisite — polymorphism.** `dynamic_cast` only works on **polymorphic types**: classes with at least one `virtual` function (very often a `virtual` destructor). That virtual-ness is what causes the compiler to embed **Run-Time Type Information (RTTI)** — a hidden record, reachable via the object's vtable, that says "I am really a `B`." `dynamic_cast` reads that record. No virtual function ⇒ no RTTI ⇒ `dynamic_cast` won't compile for that type.

**Two behaviours depending on what you cast.** This distinction is the crux of ex02:

- **On a pointer**: on failure it returns the **null pointer**. So you test a candidate and a null result tells you "not this one."
- **On a reference**: there is no "null reference," so on failure it **throws** `std::bad_cast`. You wrap the attempt in `try`/`catch` and a caught exception tells you "not this one."

ex02 deliberately asks you to implement identification for *both* a pointer and a reference so you *experience* this difference: the pointer version branches on a null check, the reference version branches on a caught exception. (And the rule "no pointer inside the reference version" forces you to use the throwing form rather than sidestepping it.)

**Why `typeinfo` is forbidden in ex02:** C++ ships an easier tool, `typeid` (from `<typeinfo>`), that just tells you an object's type name directly. Banning it forces you to solve identification *through dynamic_cast itself* — i.e. to internalize how runtime type discovery actually works, instead of calling a black box.

**Upcast vs. downcast vocabulary** (you'll use these words in defense):
- **Upcast** = derived → base (e.g. `A*` → `Base*`). Always safe, usually implicit; `dynamic_cast` allows it trivially.
- **Downcast** = base → derived (e.g. `Base*` → `A*`). This is the risky direction, and the whole reason `dynamic_cast`'s runtime check exists.

### 2.3 `reinterpret_cast<T>(expr)` — "reinterpret the raw bits"

**What it's for:** low-level conversions where you take a value's bit pattern and declare "treat this as a completely different type." The compiler does **no** value adjustment and **no** safety check — it just changes the type label on the bits. This is the most dangerous cast and should be rare.

Canonical legitimate uses:
- Pointer ↔ sufficiently large integer.
- Converting between unrelated pointer types when you genuinely know what you're doing (serialization, hardware registers, etc.).

**Why it matters here (ex01):** Serialization is *the* textbook use case. The exercise asks you to turn a pointer into a plain unsigned integer and turn it back again. There is no "logical/value" relationship between a pointer and an integer — only a **bit-level** one (the integer holds the same numeric address the pointer holds). `static_cast` would (rightly) refuse this. `reinterpret_cast` is the only cast that permits it, which is exactly why the exercise exists: to give you a situation where `reinterpret_cast` — and *only* `reinterpret_cast` — is correct.

The exercise's success criterion (deserializing a serialized pointer gives back the *original* pointer) is teaching you the key guarantee: a pointer → integer → pointer round-trip, through an integer type wide enough to hold the address, is **well-defined and lossless** — you get the *same address* back. (See §3.2 for why the *width* of that integer type matters.)

**Danger reminder:** `reinterpret_cast` lets you create pointers that point at nonsense if you feed it a garbage integer. The guarantee only holds when the integer genuinely came from a real pointer of a compatible kind.

### 2.4 `const_cast<T>(expr)` — add or remove `const`/`volatile`, nothing else

**What it's for:** the *only* cast that may change the `const` (or `volatile`) qualifier of a reference/pointer — and it may change **only** that, never the underlying type.

```cpp
const int  ci = 42;
const int* cp = &ci;
int*       p  = const_cast<int*>(cp);   // strips const
```

It isn't the focus of any exercise, but you must know it to complete your mental map of the four:
- It exists because sometimes you must pass a `const` object to an older API that (wrongly) takes a non-`const` pointer it won't actually modify.
- **Critical caveat:** *using* `const_cast` to actually **modify** an object that was *originally declared `const`* is **undefined behavior**. Removing `const` is only safe when the object underneath was never truly const to begin with.
- The takeaway for defense: "`const_cast` changes only constness; abusing it to mutate a real const object is UB."

### 2.5 Quick decision table

| You want to… | Use | Checked? |
|---|---|---|
| Convert between numeric/char types, or `void*`→`T*` | `static_cast` | compile-time |
| Safely downcast in a polymorphic hierarchy | `dynamic_cast` | **runtime** |
| Turn a pointer into an integer (or unrelated reinterpretation) | `reinterpret_cast` | none |
| Add/remove only `const`/`volatile` | `const_cast` | n/a |
| (legacy) any of the above, ambiguously | C-style `(T)x` | varies — avoid |

---

## 3. Exercise-by-exercise concept walkthrough

The point here is the *thinking*, not the code. Each section tells you what to understand and what decisions you face — it deliberately stops short of handing you signatures or step-by-step logic. Working those out *is* the exercise.

### 3.1 ex00 — ScalarConverter (the meaty one)

**Goal:** given a string like `"42.0f"`, figure out which kind of literal it is, parse it into that actual type, then convert it into the other three types and print all four results — handling impossible/overflowing/non-displayable cases.

This exercise is less about the cast itself (it's `static_cast` throughout) and more about everything *around* the cast. Concepts you need:

**(a) "A class that can't be instantiated."**
`ScalarConverter` holds no state and exposes its work through a `static` method. The subject says it "must not be instantiable by users." Think about *how* C++ lets you forbid construction, and where you'd put the constructor(s) to achieve that. You still respect the *Orthodox Canonical Form* requirement from the general rules — but in a way that *prevents* construction. Ask yourself: why is a `static` method the right shape for a stateless utility? (Because there's nothing per-object to store; the function is really just a namespaced free function.)

**(b) Literal type detection — parsing.**
Before converting, you must *classify* the input string. The four categories:
- **char**: a single *non-numeric* displayable character, conventionally shown as `'c'`. Beware the edge: a single digit like `5` is the **int** 5, not a char — only a non-digit single character is a char literal.
- **int**: optional sign + digits, no dot, no `f` (e.g. `-42`).
- **float**: has a trailing `f`, decimal notation (e.g. `4.2f`), plus the pseudo-literals `-inff`, `+inff`, `nanf`.
- **double**: has a dot but no `f` (e.g. `4.2`), plus pseudo-literals `-inf`, `+inf`, `nan`.

You'll need string inspection and a string→number conversion. The subject explicitly *authorizes* helper functions that convert string→int/float/double — but warns they "won't do the whole job," because **detection, edge cases, and formatting are on you.** Think in terms of: *classify first, then parse with the matching tool.*

**(c) The special floating-point values — this is a real concept, not a gimmick.**
`nan`, `+inf`, `-inf` are genuine IEEE-754 floating-point values, not errors:
- **infinity**: result of overflow/divide-by-zero in floating point; behaves like a value larger than everything finite.
- **NaN** ("Not a Number"): result of undefined operations like `0.0/0.0`; its defining weirdness is that `NaN != NaN` is `true`. You can't compare your way to detecting it.

Why they matter: these values have **no meaningful `char` or `int` representation**, so converting them to those types is "impossible" and you must say so. You'll reach for `<limits>` to *produce* them and `<cmath>` to *test* for them. The pseudo-literals exist specifically to make you handle the "this conversion makes no sense" branch.

**(d) Overflow and range — `std::numeric_limits`.**
A `double` can hold values far outside an `int`'s or `char`'s range. Casting `1e40` to `int` is meaningless/overflowing. So before printing the `int`/`char` results you must reason: *does the source value fit in the target type's range?* The `min()`/`max()` of `std::numeric_limits` (for `int` and for `char`) give you those bounds. This is the difference between "conversion impossible" and a real number. **This is a core skill the exercise is testing:** range-aware conversion.

**(e) Displayability of `char`.**
Even when the numeric value fits in a `char`, it might be a control character (non-printable). The subject wants `char: Non displayable` in that case. Concept: the ASCII printable range, and `std::isprint` from `<cctype>`. Distinguish three char outcomes: *impossible* (out of range / nan / inf), *non displayable* (in range but not printable), and the actual `'c'`.

**(f) Output formatting.**
Floats print with a trailing `f` and at least one decimal (`42.0f`); doubles with a decimal (`42.0`). That's a `std::cout`/`iostream` formatting concern (`std::fixed`, precision, manually appending `.0`/`f`). Minor, but graders look at it.

### 3.2 ex01 — Serializer (pointers as numbers)

**Goal:** a serialize function that turns a pointer into an unsigned integer, a deserialize function that turns it back, and a test proving the round-trip returns the identical pointer. Same non-instantiable, static-only class shape as ex00. Part of the exercise is deciding the function signatures yourself — including *which* integer type is wide enough to be safe.

Concepts:

**(a) What a pointer *is*.** A pointer is, at the machine level, just a number: the memory address of an object. Everything "pointer-y" (type, arithmetic, dereferencing) is the compiler's interpretation on top of that number.

**(b) Choosing an integer type wide enough.** Your integer must be guaranteed wide enough to hold *any* pointer value with no loss. On a 64-bit system a pointer is 64 bits, so a 32-bit `int` would truncate the address and break the round-trip. The standard library provides an integer type whose entire purpose is to be exactly pointer-width on whatever platform you compile for — finding and justifying that type is a deliberate design decision the exercise wants from you. (See the C++98 caveat in §4 about where this type comes from.)

**(c) Why `reinterpret_cast` and not `static_cast`.** There is no *value* relationship between a pointer and a number you could meaningfully add or compare in pointer terms — only a *bitwise* identity (same address bits). `static_cast` deliberately refuses pointer↔integer conversion because it's not a "sensible value conversion." `reinterpret_cast` is the sanctioned tool for "these bits, viewed as the other type." The standard guarantees the pointer→integer→pointer round-trip (through a wide-enough unsigned integer type) yields the original pointer, which is why your equality test passes.

**(d) The `Data` struct.** It must be "non-empty" (have real members). The data content is irrelevant to the cast — the exercise just wants a real object with a real address so the round-trip is meaningful. The lesson is *about the address*, not the bytes inside.

### 3.3 ex02 — Identify real type (polymorphism + RTTI)

**Goal:** a `Base` with a public **virtual destructor**; empty `A`, `B`, `C` publicly inheriting from it; a `generate()` that randomly makes one and returns a `Base*`; and two identification functions (one taking a pointer, one taking a reference) that report the real type — *without* `<typeinfo>`/`typeid`.

Concepts (this is the most theory-dense exercise):

**(a) Polymorphism, vtables, and the virtual destructor.**
Inheritance lets a `Base*` point at an `A`, `B`, or `C`. **Runtime polymorphism** means the program can, while running, act on the *actual* derived object behind a base pointer. The machinery: each polymorphic object carries a hidden pointer to its class's **vtable**, and alongside it the **RTTI** that records its true type.

The single `virtual` function that makes `Base` polymorphic here is its **destructor**. Why a *virtual destructor* in particular?
- It's the minimum needed to make the type polymorphic (so `dynamic_cast` works).
- More importantly, it's *correct design*: deleting a derived object through a `Base*` only runs the right destructors if the base destructor is virtual. Without it you'd get undefined behavior on `delete`. So this requirement is teaching a real-world rule: **base classes meant for polymorphic use need a virtual destructor.**

**(b) `dynamic_cast` as the identification mechanism.**
With `<typeinfo>` banned, the only way to discover the concrete type is to *try* each possibility and see which attempt succeeds. The two identification functions force you to confront the same idea through two different failure signals:
- The **pointer** version has to use the signal `dynamic_cast` gives when a pointer cast fails (review §2.2).
- The **reference** version can't use that signal — references have no equivalent "empty" value — so it has to use the *other* failure mechanism, which means control flow built around `try`/`catch`. The rules also forbid taking the address of the reference, so you can't dodge back into the pointer form.

Working out *which* signal goes with *which* form, and structuring the branching around each, is the headline lesson of ex02 — so it's left to you here on purpose.

**(c) Randomness.** `generate()` needs a random choice among three classes. The subject says use anything (`rand()` seeded with `time()`, etc.). It's incidental to the casting lesson — just enough nondeterminism that identification is actually doing work.

**(d) Note on Canonical Form.** The subject *exempts* these four classes from Orthodox Canonical Form — a hint that the focus is purely on the polymorphic hierarchy and dynamic_cast, not boilerplate.

---

## 4. Cross-cutting knowledge you'll lean on

These come from earlier modules but are load-bearing here. Treat as refreshers.

- **Orthodox Canonical Form (OCF):** default constructor, copy constructor, copy assignment operator, destructor. Required across modules *except where the subject exempts you* (ex02's four classes). For the utility classes (ex00/ex01) the twist is that you *also* prevent instantiation.
- **`static` members vs. instances:** a `static` member function belongs to the class, not any object — callable as `ScalarConverter::convert(...)` with no instance. Perfect for stateless utilities.
- **Header hygiene:** include guards (no double inclusion), each header self-sufficient, no function definitions in headers (templates excepted). These are graded-to-zero rules from the general section — don't lose easy points.
- **No STL containers/algorithms, C++98 only:** so no `<algorithm>`, no `nullptr` (use `NULL`/`0`), no `std::stof`/`std::to_string` (those are C++11). Use C++98-era tools (`std::stringstream`, `strtod`, etc.). `printf`/`malloc`/`free` are outright forbidden.
- **Caveat on the pointer-width integer type (ex01):** the obvious choice, `uintptr_t` from `<cstdint>`, actually entered the language in C++11 (inherited from C99), so it isn't strictly guaranteed by C++98. In practice the 42 toolchain compiles it without complaint and the subject expects this kind of type — just know that if a defense evaluator presses you on "is that C++98?", the honest answer is "it predates C++11 in C, and the compiler here provides it." `long`/`unsigned long` is *usually* but not *portably* pointer-width, which is exactly the unreliability §3.2(b) warns about.
- **Exceptions:** ex02's reference path needs `try`/`catch` and `std::exception`/`std::bad_cast`. Refresher on how a `throw` unwinds to the nearest matching `catch`.
- **Standard headers you'll likely meet:** `<limits>` (numeric_limits), `<cmath>` (isnan/isinf), `<cctype>` (isprint), `<cstdint>` (the pointer-width integer type), `<sstream>`/`<iostream>` (parsing & formatting), `<typeinfo>` (the `std::bad_cast` type lives here — note you can catch it via `std::exception` without relying on `typeid`).

---

## 5. Self-check questions (use these to test real understanding)

If you can answer these out loud, you're ready for defense:

1. Why does the module forbid the C-style cast in spirit, and what does each named cast give you that `(T)x` doesn't?
2. Place the four casts on the safe→unsafe spectrum and justify the order.
3. In ex00, which conversions could happen *implicitly*, and why use `static_cast` anyway?
4. How do you tell "impossible" from "non displayable" from a real char result?
5. Why is `NaN != NaN`, and how does that affect how you detect it?
6. In ex01, why must the integer type be pointer-width, and what breaks if it isn't? What's the catch about that type and C++98?
7. Why is `reinterpret_cast` legal for pointer↔integer but `static_cast` isn't?
8. What makes a class "polymorphic," and why does `dynamic_cast` require it?
9. Why must `Base`'s destructor be virtual — give *both* reasons.
10. How does `dynamic_cast` report failure for a pointer vs. for a reference, and why the difference?
11. Why is `<typeinfo>`/`typeid` banned in ex02 — what would it let you skip learning?

---

## 6. One-paragraph summary to anchor it all

C++ replaces C's single ambiguous cast with four intentful operators. `static_cast` performs sensible, compile-time-validated value conversions (your tool for ex00's number/char juggling). `dynamic_cast` performs runtime-checked navigation of a polymorphic hierarchy, reporting failure as null (pointers) or a thrown `std::bad_cast` (references) — the heart of ex02. `reinterpret_cast` reinterprets raw bits with no checks, the only legal way to round-trip a pointer through a pointer-width integer in ex01. `const_cast` changes only constness. Knowing *which* cast a situation demands — and being able to defend that choice — is the entire skill this module builds.
