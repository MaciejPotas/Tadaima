# Japanese Quiz Studio v6

A dependency-free Python desktop editor and simulator for XML-driven Japanese
quizzes.

Version 6 keeps language knowledge in XML:

- dictionaries contain words, forms, functional word tags, and optional
  relationships,
- sentence vocabulary contains reusable entities and reusable tag profiles,
- sentence patterns contain only a Polish question and a Japanese answer,
- contexts contain synchronized Polish and Japanese prefixes,
- Python implements generic token, profile, relationship, tag, dictionary,
  and form matching.

There are no verb IDs, food rules, destination rules, adjective maps, sentence
rules, slot declarations, copied form variants, or semantic profile names in
the Python code.

## Requirements and start

- Python 3.9 or newer
- Tkinter, normally included with Python on Windows and macOS

No pip packages are required.

```bash
python app.py
```

To open another project:

```bash
python app.py path/to/quiz_project.xml
```

On Windows:

```text
run_windows.bat
```

Command-line validation:

```bash
python app.py --validate
```

## Included files

```text
examples/verbs.xml
examples/adjectives.xml
examples/copulas.xml
examples/sentence_vocabulary.xml
examples/contexts.xml
examples/sentence_maps.xml
examples/quiz_project.xml
```

The bundled project contains 44 verbs, 84 adjectives, one copula, 31 reusable
entities, 19 tag profiles, and 11 sentence patterns.

## Optional relationships

A relationship describes a capability of an entity that a verb can use. It
never says that the role is required.

```xml
<word
    id="taberu"
    kana="たべる"
    kanji="食べる"
    translation="jeść"
    type="ichidan"
    ending="ru">
  <relations>
    <relation role="object">
      <tag>edible</tag>
    </relation>
  </relations>
  <forms>
    ...
  </forms>
</word>
```

The relationship means:

> If a selected pattern asks for `object`, `taberu` can use an entity whose
> effective tags include `edible`.

It does not mean that `taberu` always needs an object. The word editor uses a
compact format:

```text
object: edible
destination: land_destination
companion: person
```

The relation tags describe usable capabilities rather than broad classes.
For example, `tateru` asks for `buildable`, while `kau` asks for
`purchasable`. A building may provide both capabilities; a motorcycle may be
purchasable without being buildable by that sentence pattern.

## Tag profiles reduce entity maintenance

Entities no longer need to repeat long tag lists. A profile declares a reusable
bundle and may include another profile:

```xml
<profile id="vehicle">
  <includes>
    <profile ref="physical_object" />
  </includes>
  <tags>
    <tag>speed</tag>
    <tag>safety</tag>
    <tag>rideable</tag>
    <tag>washable</tag>
  </tags>
</profile>
```

Adding a motorcycle then needs only its language data and one profile:

```xml
<entity id="motorcycle" kana="バイク">
  <polish
      gender="masculine"
      nominative="motocykl"
      accusative="motocykl"
      genitive="motocykla"
      locative="motocyklu"
      instrumental="motocyklem"
      demonstrative="ten motocykl" />
  <profiles>
    <profile ref="vehicle" />
  </profiles>
</entity>
```

The inherited profile supplies tags such as `color`, `age_new_old`,
`purchasable`, and `rideable`. Explicit entity tags are still supported for
real exceptions. For example, a bag uses `portable_object` and adds only
`washable`.

The profile IDs and their meanings exist only in XML. The Python engine merely
expands references and compares resulting tags, so adding or renaming a
semantic group does not require a code change.

## Bare sentences ignore relationships

The simple pattern contains no entity role:

```xml
<sentence_pattern id="simple">
  <question>{context.pl_prefix}{verb.prompt}.</question>
  <answer>{context.jp_prefix}{verb.form}</answer>
</sentence_pattern>
```

Every verb with the selected form can use it:

```text
きのう、はなした
いま、はなしている
きのう、たべた
あした、いく
```

No object, companion, or destination is requested, so no relationship is
checked.

## Patterns decide which roles are needed

An object pattern contains `{object}`:

```xml
<sentence_pattern id="transitive_object">
  <question>
    {context.pl_prefix}{verb.prompt} {object.pl_accusative}.
  </question>
  <answer>
    {context.jp_prefix}{object}を{verb.form}
  </answer>
</sentence_pattern>
```

The generic matching process is:

1. Detect `verb` and `object` from the pattern.
2. Keep verbs with an `object` relationship.
3. Expand each entity's profiles into effective tags.
4. Match the relationship tags with the effective entity tags.
5. Apply the selected verb form and its context.
6. Render the two pattern strings.

For `taberu`, an apple can be selected because the verb requests `edible` and
the apple inherits that tag from the `fruit` profile.

## Adjectives use the same tag bank

An adjective carries descriptive tags:

```xml
<word id="oishii" kana="おいしい" type="i_adjective">
  <tags>
    <tag>taste</tag>
  </tags>
  <forms>
    ...
  </forms>
</word>
```

`oishii` can describe an entity whose effective tags include `taste`.
`atarashii`, tagged `age_new_old`, can describe a newly added motorcycle
without copying `age_new_old` onto that entity because `vehicle` inherits it
from `physical_object`.

```xml
<sentence_pattern id="described_object_action">
  <question>
    Ułóż zdanie: {context.pl_prefix}{verb.prompt};
    {adjective.prompt} {object.pl_nominative}.
  </question>
  <answer>
    {context.jp_prefix}{adjective.attributive}{object}を{verb.form}
  </answer>
</sentence_pattern>
```

The simulator can optionally filter this slot to:

```text
<any adjective>
i_adjective
na_adjective
```

`{adjective.attributive}` reads the stored attributive form, so both adjective
types work in one pattern:

```text
おいしいりんご
しずかなおんがく
```

## New roles require no Python change

Any entity token can be a relationship role. For example:

```xml
<relation role="vehicle">
  <tag>rideable</tag>
</relation>
```

can be used by:

```xml
<answer>{vehicle}に{verb.form}</answer>
```

The renderer reads `vehicle` directly from the token and XML relationship. It
does not have a programmed list containing object, destination, companion, or
vehicle.

If a pattern has multiple verbs that could provide the same role, make the
intended verb the dynamic `.form` target or use distinct role names. The editor
reports an ambiguity instead of guessing.

## Word-slot filters

Descriptive word slot names can filter functional word tags:

```text
{reporting_verb.form}          requires tag: reporting
{progressive_auxiliary.form}   requires tag: progressive_auxiliary
```

These are the only top-level verb tags needed by the bundled patterns. Semantic
verb tags such as `food_consumption`, `motion`, and `creation` were removed
because relationships now express the usable capability directly.

Names beginning with `verb_` are unfiltered verb slots:

```text
{verb_before.dictionary}
{verb_main.form}
```

This lets a pattern contain two ordinary verbs without binding either one to a
specific dictionary ID.

## Forms

`.form` identifies the one word controlled by the quiz form selector:

```text
{verb.form}
{verb_main.form}
{reporting_verb.form}
{progressive_auxiliary.form}
{adjective.form}
{copula.form}
```

The default selector exposes the eight finite forms stored by the selected
dictionary. Verbs and adjectives retain their additional future-ready forms.

Fixed grammar selectors include:

```text
{adjective.attributive}
{verb.dictionary}
{verb.te}
{predicate.plain_nonpast}
{verb.potential}
{verb.passive}
{verb.causative}
{adjective.conditional}
```

## Adjective patterns and the copula

The three bundled adjective patterns have distinct jobs:

```xml
<sentence_pattern id="adjective_predicate">
  <question>{subject.pl_demonstrative} {adjective.prompt}.</question>
  <answer>この{subject}は{adjective.form}</answer>
</sentence_pattern>

<sentence_pattern id="adjective_simple">
  <question>To {adjective.prompt}.</question>
  <answer>これは{adjective.form}</answer>
</sentence_pattern>

<sentence_pattern id="adjective_attributive">
  <question>
    To {copula.prompt} {adjective.prompt} {subject.pl_nominative}.
  </question>
  <answer>
    これは{adjective.attributive}{subject}{copula.form}
  </answer>
</sentence_pattern>
```

`adjective.form` selects the complete i-adjective or na-adjective predicate
form. The before-noun pattern instead selects the stored
`attributive_nonpast` form and a finite form from `copulas.xml`.

Python does not generate `この`, `だ`, or `です`. `この` is literal pattern
text, while the copula values come from the declared `copulas` dictionary.

## Extended live example

The pattern editor updates while typing and shows:

- the rendered Polish question,
- the expected Japanese answer,
- the dynamic target and selected form,
- the selected context,
- every selected word and entity,
- each entity's profile expansion,
- the exact shared tags that made each relationship valid.

For example:

```text
object=ringo uses profile(s): fruit
verb=taberu can use object=ringo through shared tag(s): edible
adjective=oishii describes ringo through shared tag(s): taste
```

This makes it possible to diagnose an unexpected sentence without reading the
Python code.

## Possibility counter

The simulator has a **Count possibilities** button. It respects:

- the selected sentence pattern,
- one selected form or all random forms,
- the word and entity dropdowns explicitly selected since the last refresh,
- the optional adjective type filter,
- available stored forms,
- verb-to-entity relationships,
- expanded entity profiles,
- entity-to-adjective shared tags,
- context pools.

It reports:

- possible generated sentences,
- vocabulary combinations,
- unique Japanese answers,
- unique Polish questions,
- number of quiz forms and context options,
- eligible verbs, adjectives, and entities for every slot,
- compatible verb-to-entity and entity-to-adjective pairs.

Context weights affect selection probability. They do not multiply the number
of distinct possibilities.

## Simulator dropdowns

After selecting a pattern and quiz form, the simulator creates one dropdown
for every detected word or entity slot. For example:

```text
{context.jp_prefix}{adjective.attributive}{object}を{verb.form}
```

creates `adjective`, `object`, and `verb` menus.

All menus begin blank. Their lists contain the compatible dictionary words or
reusable vocabulary entities. Every item explicitly selected by the user
remains a constraint until **Refresh** is pressed:

- selecting `verb=taberu` filters the object menu to edible entities,
- selecting `object=book` filters the verb menu to verbs that can use a book,
- adjective choices follow the effective tags of compatible entities,
- the remaining unselected fields are filled with one random compatible
  completion,
- selecting an adjective after selecting a verb preserves that verb and adds
  the adjective as another constraint.

The selection direction is not hardcoded. The same XML relationships and tags
are projected in either direction. The **Refresh** button clears all saved
selections and returns every menu to its blank default state.

**Generate preview** produces one question and one expected Japanese answer.
The build trace remains visible and explains the selected forms, context,
relationships, tags, and token replacements.

## Stable IDs

Dictionary IDs use lowercase romaji:

```text
yomu
kaku
taberu
oishii
aoi
shizuka
```

Kana and kanji remain separate display values. An optional semantic suffix can
distinguish equal readings:

```text
kiru_wear
kiru_cut
kaeru_return
kaeru_change
```

## Editor tabs

- **Words** edits verbs, adjectives, functional word tags, optional
  relationships, and stored forms.
- **Sentence vocabulary / Entities** edits reusable entities, Polish forms,
  selected profiles, and exceptional tags.
- **Sentence vocabulary / Tag profiles** edits profile inheritance and
  reusable tag bundles.
- **Sentence maps** edits pure question and answer patterns, displays the live
  example, and includes token help.
- **Contexts** edits synchronized Polish and Japanese context alternatives.
- **Simulator** provides dependent slot dropdowns, preserves selections until
  refresh, generates one preview, displays a build trace, and counts exact
  possibilities.
