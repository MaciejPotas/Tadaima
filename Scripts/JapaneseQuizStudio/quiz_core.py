"""Token-driven XML model and renderer for Japanese Quiz Studio v6."""

from __future__ import annotations

from dataclasses import dataclass, field
from itertools import product
from pathlib import Path
import random
import re
from typing import Dict, Iterable, Iterator, List, Mapping, Optional
import xml.etree.ElementTree as ET


TOKEN_PATTERN = re.compile(r"\{([^{}]+)\}")
POLISH_FIELDS = (
    "nominative",
    "accusative",
    "genitive",
    "locative",
    "instrumental",
    "demonstrative",
)
PROMPT_GENDERS = {
    "masculine": "prompt_m",
    "feminine": "prompt_f",
    "neuter": "prompt_n",
    "plural": "prompt_p",
}
WORD_METADATA_FIELDS = {
    "id",
    "kana",
    "kanji",
    "translation",
    "type",
    "ending",
    "categories",
    "tags",
    "prompt",
    "style",
    "form_name",
}
ENTITY_FIELDS = {
    "id",
    "kana",
    "kanji",
    "gender",
    "profiles",
    "categories",
    "tags",
    *(f"pl_{name}" for name in POLISH_FIELDS),
}
VERB_FORM_ORDER = (
    "dictionary",
    "polite_nonpast",
    "plain_negative",
    "polite_negative",
    "past_plain",
    "past_polite",
    "past_negative_plain",
    "past_negative_polite",
    "te_form",
    "potential",
    "passive",
    "causative",
    "tara",
    "volitional",
    "imperative",
)
ADJECTIVE_FORM_ORDER = (
    "predicate_plain_nonpast",
    "predicate_polite_nonpast",
    "predicate_plain_negative",
    "predicate_polite_negative",
    "predicate_plain_past",
    "predicate_polite_past",
    "predicate_plain_past_negative",
    "predicate_polite_past_negative",
    "attributive_nonpast",
    "connective",
    "adverbial",
    "conditional",
    "nominalized",
    "appearance_sou",
    "excessive_sugiru",
)


class ProjectError(ValueError):
    """Raised when project data or a rendering request is invalid."""


@dataclass
class Form:
    name: str
    value: str
    prompt: str = ""
    context: str = "none"
    style: str = ""
    prompt_m: str = ""
    prompt_f: str = ""
    prompt_n: str = ""
    prompt_p: str = ""

    def prompt_for(self, gender: str = "") -> str:
        field_name = PROMPT_GENDERS.get(gender, "")
        if field_name:
            value = getattr(self, field_name)
            if value:
                return value
        return self.prompt


@dataclass
class Word:
    dictionary_id: str
    id: str
    translation: str
    kana: str
    kanji: str = ""
    type: str = ""
    ending: str = ""
    categories: List[str] = field(default_factory=list)
    relations: Dict[str, List[str]] = field(default_factory=dict)
    forms: Dict[str, Form] = field(default_factory=dict)

    @property
    def tags(self) -> List[str]:
        """Compatibility alias for older integrations."""
        return self.categories


@dataclass
class TagProfile:
    id: str
    includes: List[str] = field(default_factory=list)
    tags: List[str] = field(default_factory=list)


@dataclass
class Entity:
    id: str
    kana: str
    kanji: str = ""
    gender: str = ""
    profiles: List[str] = field(default_factory=list)
    categories: List[str] = field(default_factory=list)
    polish: Dict[str, str] = field(default_factory=dict)

    @property
    def tags(self) -> List[str]:
        """Compatibility alias for older integrations."""
        return self.categories


@dataclass
class SentencePattern:
    id: str
    label: str
    question: str
    answer: str

    def slot_names(self) -> List[str]:
        return pattern_slot_names(self.question + "\n" + self.answer)


@dataclass
class ContextOption:
    id: str
    pl_prefix: str = ""
    jp_prefix: str = ""
    weight: int = 1


@dataclass
class ContextPool:
    id: str
    label: str
    options: Dict[str, ContextOption] = field(default_factory=dict)

    def choose(self, rng: random.Random) -> ContextOption:
        values = list(self.options.values())
        if not values:
            raise ProjectError(f"Context pool '{self.id}' has no options.")
        return rng.choices(
            values,
            weights=[max(1, value.weight) for value in values],
            k=1,
        )[0]


@dataclass
class PatternAnalysis:
    word_slots: List[str]
    entity_slots: List[str]
    dynamic_slot: str
    primary_specs: Dict[str, str]
    driver_for_entity: Dict[str, str]
    entity_for_adjective: Dict[str, str]


@dataclass
class CompatibleChoices:
    """Current valid menu items for every configurable pattern slot."""

    form_names: List[str]
    words: Dict[str, List[Word]]
    entities: Dict[str, List[Entity]]


@dataclass
class CompletedChoices:
    """One complete, mutually compatible simulator selection."""

    form_name: str
    words: Dict[str, str]
    entities: Dict[str, str]


@dataclass
class Preview:
    source: str
    answer: str
    pattern_id: str
    dynamic_slot: str
    dynamic_form: str
    words: Dict[str, str]
    forms: Dict[str, str]
    entities: Dict[str, str]
    context_pool: str
    context_option: str
    replacements: Dict[str, str]
    matches: List[str] = field(default_factory=list)

    @property
    def word_id(self) -> str:
        return self.words.get(self.dynamic_slot, next(iter(self.words.values()), ""))

    @property
    def form_name(self) -> str:
        return self.dynamic_form

    def trace(self) -> str:
        lines = [
            f"Sentence pattern: {self.pattern_id}",
            f"Quiz target: {self.dynamic_slot or '(fixed forms only)'}",
            f"Selected form: {self.dynamic_form or '(fixed)'}",
            f"Context: {self.context_pool} / {self.context_option}",
        ]
        if self.words:
            lines.append(
                "Words: "
                + ", ".join(
                    f"{slot}={word_id} [{self.forms.get(slot, '')}]"
                    for slot, word_id in self.words.items()
                )
            )
        if self.entities:
            lines.append(
                "Entities: "
                + ", ".join(
                    f"{slot}={entity_id}"
                    for slot, entity_id in self.entities.items()
                )
            )
        if self.matches:
            lines.extend(("", "Compatibility"))
            lines.extend(f"  {value}" for value in self.matches)
        lines.extend(("", "Replacements"))
        for token, value in sorted(self.replacements.items()):
            lines.append(f"  {token} = {value}")
        return "\n".join(lines)

    def live_example(self) -> str:
        """Return a compact but explanatory pattern-editor preview."""
        lines = [
            f"Question: {self.source}",
            f"Answer:   {self.answer}",
            "",
            f"Target: {self.dynamic_slot or 'fixed'}",
            f"Form: {self.dynamic_form or 'fixed'}",
            f"Context: {self.context_pool} / {self.context_option}",
        ]
        for slot_id, word_id in self.words.items():
            lines.append(
                f"{slot_id}: {word_id} ({self.forms.get(slot_id, 'fixed')})"
            )
        for slot_id, entity_id in self.entities.items():
            lines.append(f"{slot_id}: {entity_id}")
        if self.matches:
            lines.append("")
            lines.append("Why these items match:")
            lines.extend(f"• {value}" for value in self.matches)
        return "\n".join(lines)


@dataclass
class PossibilityStats:
    """Exact counts for one simulator configuration."""

    pattern_id: str
    vocabulary_combinations: int
    generated_sentences: int
    unique_questions: int
    unique_answers: int
    form_names: List[str]
    context_options: List[str]
    word_counts: Dict[str, int]
    entity_counts: Dict[str, int]
    connection_counts: Dict[str, int]

    def summary(self) -> str:
        lines = [
            f"Possible generated sentences: {self.generated_sentences:,}",
            f"Vocabulary combinations: {self.vocabulary_combinations:,}",
            f"Unique Japanese answers: {self.unique_answers:,}",
            f"Unique Polish questions: {self.unique_questions:,}",
            f"Quiz forms ({len(self.form_names):,}): "
            + ", ".join(self.form_names),
            f"Context options ({len(self.context_options):,}): "
            + ", ".join(self.context_options),
        ]
        if self.word_counts or self.entity_counts:
            lines.extend(("", "Eligible items"))
            for slot_id, count in self.word_counts.items():
                is_adjective = _slot_kind(slot_id) == "adjective"
                kind = (
                    "adjective" if count == 1 else "adjectives"
                ) if is_adjective else (
                    "word" if count == 1 else "words"
                )
                lines.append(f"  {slot_id}: {count:,} {kind}")
            for slot_id, count in self.entity_counts.items():
                kind = "entity" if count == 1 else "entities"
                lines.append(f"  {slot_id}: {count:,} {kind}")
        if self.connection_counts:
            lines.extend(("", "Compatible pairs"))
            for connection, count in self.connection_counts.items():
                lines.append(f"  {connection}: {count:,}")
        lines.extend(
            (
                "",
                "Context weights affect random probability, not the number "
                "of distinct possibilities.",
            )
        )
        return "\n".join(lines)


def _required_attr(node: ET.Element, name: str, location: str) -> str:
    value = (node.get(name) or "").strip()
    if not value:
        raise ProjectError(f"Missing attribute '{name}' in {location}.")
    return value


def _parse_csv(value: str) -> List[str]:
    return [part.strip() for part in value.split(",") if part.strip()]


def _write_xml(root: ET.Element, path: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    tree = ET.ElementTree(root)
    ET.indent(tree, space="  ")
    tree.write(path, encoding="utf-8", xml_declaration=True)


def _load_categories(node: ET.Element) -> List[str]:
    categories = [
        (item.text or "").strip()
        for item in node.findall("./categories/category")
        if (item.text or "").strip()
    ]
    if categories:
        return categories
    return [
        (item.text or "").strip()
        for item in node.findall("./tags/tag")
        if (item.text or "").strip()
    ]


def _load_relations(node: ET.Element) -> Dict[str, List[str]]:
    """Load optional role relationships without interpreting their names."""
    result: Dict[str, List[str]] = {}
    relation_nodes = list(node.findall("./relations/relation"))
    relation_nodes.extend(node.findall("./accepts"))
    for relation in relation_nodes:
        role = _required_attr(relation, "role", "word relation")
        tags = [
            (tag.text or "").strip()
            for tag in relation.findall("tag")
            if (tag.text or "").strip()
        ]
        tags.extend(_parse_csv(relation.get("tags") or ""))
        unique_tags = result.setdefault(role, [])
        for tag in tags:
            if tag not in unique_tags:
                unique_tags.append(tag)
    return result


def pattern_slot_names(text: str) -> List[str]:
    """Return unique non-context token roots in first-appearance order."""
    result: List[str] = []
    for expression in TOKEN_PATTERN.findall(text):
        root = expression.split(".", 1)[0].strip()
        if root and root != "context" and root not in result:
            result.append(root)
    return result


def _token_parts(expression: str) -> tuple[str, str]:
    if "." in expression:
        root, field_name = expression.split(".", 1)
        return root.strip(), field_name.strip()
    return expression.strip(), ""


def _slot_kind(root: str) -> str:
    if root in {"adjective", "predicate", "i_adjective", "na_adjective"}:
        return "adjective"
    if (
        root == "verb"
        or root.startswith("verb_")
        or root.endswith("_verb")
        or root.endswith("_auxiliary")
    ):
        return "verb"
    # This is the name-based fallback. QuizProject additionally promotes a
    # root to a generic word slot when its name matches a declared dictionary,
    # including singular "copula" for dictionary "copulas".
    return "entity"


def _word_slot_filter_tag(slot_id: str) -> str:
    """Derive an optional word-tag filter from a descriptive slot name."""
    if slot_id.endswith("_verb") and not slot_id.startswith("verb_"):
        return slot_id[: -len("_verb")]
    if slot_id.endswith("_auxiliary"):
        return slot_id
    return ""


def _canonical_form_name(word: Word, spec: str, dynamic_form: str) -> str:
    if spec == "form":
        if not dynamic_form:
            raise ProjectError(
                f"Pattern requires a quiz form for word slot '{word.id}'."
            )
        return dynamic_form
    if spec in word.forms:
        return spec
    if spec == "te":
        return "te_form"
    if spec == "attributive":
        return "attributive_nonpast"
    adjective_aliases = {
        "plain_nonpast": "predicate_plain_nonpast",
        "polite_nonpast": "predicate_polite_nonpast",
        "plain_negative": "predicate_plain_negative",
        "polite_negative": "predicate_polite_negative",
        "plain_past": "predicate_plain_past",
        "past_plain": "predicate_plain_past",
        "polite_past": "predicate_polite_past",
        "past_polite": "predicate_polite_past",
        "plain_past_negative": "predicate_plain_past_negative",
        "past_negative_plain": "predicate_plain_past_negative",
        "polite_past_negative": "predicate_polite_past_negative",
        "past_negative_polite": "predicate_polite_past_negative",
    }
    verb_aliases = {
        "plain_nonpast": "dictionary",
        "plain_past": "past_plain",
        "polite_past": "past_polite",
        "plain_past_negative": "past_negative_plain",
        "polite_past_negative": "past_negative_polite",
    }
    aliases = adjective_aliases if word.dictionary_id == "adjectives" else verb_aliases
    return aliases.get(spec, spec)


def _word_form(
    word: Word,
    spec: str,
    dynamic_form: str,
    slot_id: str,
) -> Form:
    name = _canonical_form_name(word, spec, dynamic_form)
    form = word.forms.get(name)
    if form is None:
        raise ProjectError(
            f"Word '{word.id}' has no form '{name}' required by "
            f"slot '{slot_id}'."
        )
    return form


def _has_form(word: Word, spec: str, dynamic_form: str) -> bool:
    try:
        name = _canonical_form_name(word, spec, dynamic_form)
    except ProjectError:
        return False
    return name in word.forms


def _intersects(values: Iterable[str], accepted: Iterable[str]) -> bool:
    return bool(set(values) & set(accepted))


class QuizProject:
    """Complete editable v6 quiz project."""

    def __init__(self) -> None:
        self.manifest_path: Optional[Path] = None
        self.dictionary_paths: Dict[str, Path] = {}
        self.vocabulary_path: Optional[Path] = None
        self.maps_path: Optional[Path] = None
        self.contexts_path: Optional[Path] = None
        self.words: Dict[str, Dict[str, Word]] = {}
        self.tag_profiles: Dict[str, TagProfile] = {}
        self.entities: Dict[str, Entity] = {}
        self.patterns: Dict[str, SentencePattern] = {}
        self.contexts: Dict[str, ContextPool] = {}

        # Compatibility aliases for earlier editor integrations.
        self.templates = self.patterns

    @classmethod
    def load(cls, manifest_path: Path | str) -> "QuizProject":
        project = cls()
        project.manifest_path = Path(manifest_path).resolve()
        if not project.manifest_path.exists():
            raise ProjectError(f"Project file does not exist: {project.manifest_path}")
        try:
            root = ET.parse(project.manifest_path).getroot()
        except (OSError, ET.ParseError) as exception:
            raise ProjectError(f"Invalid project XML: {exception}") from exception
        if root.tag != "quiz_project":
            raise ProjectError("The project file must have a <quiz_project> root.")

        base = project.manifest_path.parent
        dictionaries = root.find("dictionaries")
        if dictionaries is None:
            raise ProjectError("The project manifest has no dictionaries.")
        for node in dictionaries.findall("dictionary"):
            dictionary_id = _required_attr(node, "id", "project dictionary")
            project.dictionary_paths[dictionary_id] = (
                base / _required_attr(node, "file", dictionary_id)
            ).resolve()

        vocabulary = root.find("vocabulary")
        contexts = root.find("contexts")
        maps = root.find("sentence_maps")
        if vocabulary is None or contexts is None or maps is None:
            raise ProjectError(
                "The project must reference vocabulary, contexts, and "
                "sentence maps."
            )
        project.vocabulary_path = (
            base / _required_attr(vocabulary, "file", "vocabulary")
        ).resolve()
        project.contexts_path = (
            base / _required_attr(contexts, "file", "contexts")
        ).resolve()
        project.maps_path = (
            base / _required_attr(maps, "file", "sentence maps")
        ).resolve()

        for dictionary_id, path in project.dictionary_paths.items():
            project.words[dictionary_id] = project._load_dictionary(
                dictionary_id,
                path,
            )
        project._load_vocabulary(project.vocabulary_path)
        project._load_contexts(project.contexts_path)
        project._load_patterns(project.maps_path)
        return project

    def _load_dictionary(self, dictionary_id: str, path: Path) -> Dict[str, Word]:
        try:
            root = ET.parse(path).getroot()
        except (OSError, ET.ParseError) as exception:
            raise ProjectError(
                f"Could not load dictionary '{dictionary_id}': {exception}"
            ) from exception
        if root.tag != "dictionary":
            raise ProjectError(
                f"Dictionary '{dictionary_id}' must have a <dictionary> root."
            )
        words: Dict[str, Word] = {}
        for node in root.findall("./words/word"):
            word_id = _required_attr(node, "id", f"dictionary '{dictionary_id}'")
            if word_id in words:
                raise ProjectError(
                    f"Duplicate word id '{word_id}' in '{dictionary_id}'."
                )
            word = Word(
                dictionary_id=dictionary_id,
                id=word_id,
                translation=(node.get("translation") or "").strip(),
                kana=(node.get("kana") or "").strip(),
                kanji=(node.get("kanji") or "").strip(),
                type=(node.get("type") or "").strip(),
                ending=(node.get("ending") or "").strip(),
                categories=_load_categories(node),
                relations=_load_relations(node),
            )
            for form_node in node.findall("./forms/form"):
                name = _required_attr(form_node, "name", f"word '{word_id}' form")
                word.forms[name] = Form(
                    name=name,
                    value=(form_node.text or "").strip(),
                    prompt=(form_node.get("prompt") or "").strip(),
                    context=(form_node.get("context") or "none").strip(),
                    style=(form_node.get("style") or "").strip(),
                    prompt_m=(form_node.get("prompt_m") or "").strip(),
                    prompt_f=(form_node.get("prompt_f") or "").strip(),
                    prompt_n=(form_node.get("prompt_n") or "").strip(),
                    prompt_p=(form_node.get("prompt_p") or "").strip(),
                )
            words[word_id] = word
        return words

    def _load_vocabulary(self, path: Path) -> None:
        try:
            root = ET.parse(path).getroot()
        except (OSError, ET.ParseError) as exception:
            raise ProjectError(f"Could not load vocabulary: {exception}") from exception
        if root.tag != "sentence_vocabulary":
            raise ProjectError("Vocabulary must use <sentence_vocabulary>.")
        for node in root.findall("./tag_profiles/profile"):
            profile_id = _required_attr(node, "id", "tag profile")
            if profile_id in self.tag_profiles:
                raise ProjectError(
                    f"Duplicate tag profile id '{profile_id}'."
                )
            includes = [
                (item.get("ref") or item.text or "").strip()
                for item in node.findall("./includes/profile")
                if (item.get("ref") or item.text or "").strip()
            ]
            self.tag_profiles[profile_id] = TagProfile(
                id=profile_id,
                includes=includes,
                tags=_load_categories(node),
            )
        for node in root.findall("./entities/entity"):
            entity_id = _required_attr(node, "id", "vocabulary entity")
            polish_node = node.find("polish")
            polish = {
                name: (polish_node.get(name) or "").strip()
                for name in POLISH_FIELDS
            } if polish_node is not None else {}
            self.entities[entity_id] = Entity(
                id=entity_id,
                kana=(node.get("kana") or "").strip(),
                kanji=(node.get("kanji") or "").strip(),
                gender=(polish_node.get("gender") or "").strip()
                if polish_node is not None
                else "",
                profiles=[
                    (item.get("ref") or item.text or "").strip()
                    for item in node.findall("./profiles/profile")
                    if (item.get("ref") or item.text or "").strip()
                ],
                categories=_load_categories(node),
                polish=polish,
            )

    def _load_contexts(self, path: Path) -> None:
        try:
            root = ET.parse(path).getroot()
        except (OSError, ET.ParseError) as exception:
            raise ProjectError(f"Could not load contexts: {exception}") from exception
        if root.tag != "contexts":
            raise ProjectError("Contexts must use a <contexts> root.")
        for node in root.findall("context"):
            context_id = _required_attr(node, "id", "context pool")
            pool = ContextPool(
                id=context_id,
                label=(node.get("label") or context_id).strip(),
            )
            for option_node in node.findall("option"):
                option_id = _required_attr(
                    option_node,
                    "id",
                    f"context '{context_id}' option",
                )
                try:
                    weight = int(option_node.get("weight") or "1")
                except ValueError as exception:
                    raise ProjectError(
                        f"Invalid weight in '{context_id}/{option_id}'."
                    ) from exception
                pool.options[option_id] = ContextOption(
                    id=option_id,
                    pl_prefix=option_node.get("pl_prefix") or "",
                    jp_prefix=option_node.get("jp_prefix") or "",
                    weight=weight,
                )
            self.contexts[context_id] = pool

    def _load_patterns(self, path: Path) -> None:
        try:
            root = ET.parse(path).getroot()
        except (OSError, ET.ParseError) as exception:
            raise ProjectError(
                f"Could not load sentence patterns: {exception}"
            ) from exception
        if root.tag != "sentence_maps":
            raise ProjectError("Sentence maps must use a <sentence_maps> root.")
        nodes = root.findall("./sentence_patterns/sentence_pattern")
        if not nodes:
            nodes = root.findall("./templates/template")
        for node in nodes:
            pattern_id = _required_attr(node, "id", "sentence pattern")
            self.patterns[pattern_id] = SentencePattern(
                id=pattern_id,
                label=(node.get("label") or pattern_id).strip(),
                question=(node.findtext("question") or "").strip(),
                answer=(node.findtext("answer") or "").strip(),
            )

    def save(self) -> None:
        if (
            not self.manifest_path
            or not self.vocabulary_path
            or not self.maps_path
            or not self.contexts_path
        ):
            raise ProjectError("The project has no output paths.")
        self._save_manifest()
        for dictionary_id, path in self.dictionary_paths.items():
            self._save_dictionary(dictionary_id, path)
        self._save_vocabulary(self.vocabulary_path)
        self._save_contexts(self.contexts_path)
        self._save_patterns(self.maps_path)

    def _relative(self, path: Path) -> str:
        assert self.manifest_path is not None
        try:
            return str(path.relative_to(self.manifest_path.parent))
        except ValueError:
            return str(path)

    def _save_manifest(self) -> None:
        assert (
            self.manifest_path
            and self.vocabulary_path
            and self.contexts_path
            and self.maps_path
        )
        root = ET.Element("quiz_project", {"schema_version": "6"})
        dictionaries = ET.SubElement(root, "dictionaries")
        for dictionary_id in sorted(self.dictionary_paths):
            ET.SubElement(
                dictionaries,
                "dictionary",
                {
                    "id": dictionary_id,
                    "file": self._relative(self.dictionary_paths[dictionary_id]),
                },
            )
        ET.SubElement(
            root,
            "vocabulary",
            {"file": self._relative(self.vocabulary_path)},
        )
        ET.SubElement(
            root,
            "contexts",
            {"file": self._relative(self.contexts_path)},
        )
        ET.SubElement(
            root,
            "sentence_maps",
            {"file": self._relative(self.maps_path)},
        )
        _write_xml(root, self.manifest_path)

    def _save_dictionary(self, dictionary_id: str, path: Path) -> None:
        root = ET.Element("dictionary", {"schema_version": "6", "id": dictionary_id})
        words_node = ET.SubElement(root, "words")
        for word in sorted(
            self.words.get(dictionary_id, {}).values(),
            key=lambda value: value.id,
        ):
            attributes = {
                "id": word.id,
                "translation": word.translation,
                "kana": word.kana,
                "kanji": word.kanji,
                "type": word.type,
                "ending": word.ending,
            }
            word_node = ET.SubElement(
                words_node,
                "word",
                {key: value for key, value in attributes.items() if value},
            )
            if word.categories:
                categories = ET.SubElement(word_node, "tags")
                for category in word.categories:
                    ET.SubElement(categories, "tag").text = category
            if word.relations:
                relations = ET.SubElement(word_node, "relations")
                for role, tags in word.relations.items():
                    relation = ET.SubElement(
                        relations,
                        "relation",
                        {"role": role},
                    )
                    for tag in tags:
                        ET.SubElement(relation, "tag").text = tag
            forms = ET.SubElement(word_node, "forms")
            for form in word.forms.values():
                form_attributes = {
                    "name": form.name,
                    "prompt": form.prompt,
                    "context": form.context,
                    "style": form.style,
                    "prompt_m": form.prompt_m,
                    "prompt_f": form.prompt_f,
                    "prompt_n": form.prompt_n,
                    "prompt_p": form.prompt_p,
                }
                node = ET.SubElement(
                    forms,
                    "form",
                    {
                        key: value
                        for key, value in form_attributes.items()
                        if value
                    },
                )
                node.text = form.value
        _write_xml(root, path)

    def _save_vocabulary(self, path: Path) -> None:
        root = ET.Element("sentence_vocabulary", {"schema_version": "6"})
        profiles = ET.SubElement(root, "tag_profiles")
        for profile in self.tag_profiles.values():
            node = ET.SubElement(profiles, "profile", {"id": profile.id})
            if profile.includes:
                includes = ET.SubElement(node, "includes")
                for included in profile.includes:
                    ET.SubElement(
                        includes,
                        "profile",
                        {"ref": included},
                    )
            if profile.tags:
                tags = ET.SubElement(node, "tags")
                for tag in profile.tags:
                    ET.SubElement(tags, "tag").text = tag
        entities = ET.SubElement(root, "entities")
        for entity in sorted(self.entities.values(), key=lambda value: value.id):
            node = ET.SubElement(
                entities,
                "entity",
                {
                    key: value
                    for key, value in {
                        "id": entity.id,
                        "kana": entity.kana,
                        "kanji": entity.kanji,
                    }.items()
                    if value
                },
            )
            polish = {"gender": entity.gender}
            polish.update(
                {name: entity.polish.get(name, "") for name in POLISH_FIELDS}
            )
            ET.SubElement(
                node,
                "polish",
                {key: value for key, value in polish.items() if value},
            )
            if entity.profiles:
                profiles = ET.SubElement(node, "profiles")
                for profile_id in entity.profiles:
                    ET.SubElement(
                        profiles,
                        "profile",
                        {"ref": profile_id},
                    )
            if entity.categories:
                categories = ET.SubElement(node, "tags")
                for category in entity.categories:
                    ET.SubElement(categories, "tag").text = category
        _write_xml(root, path)

    def _save_contexts(self, path: Path) -> None:
        root = ET.Element("contexts", {"schema_version": "6"})
        for pool in self.contexts.values():
            node = ET.SubElement(
                root,
                "context",
                {"id": pool.id, "label": pool.label},
            )
            for option in pool.options.values():
                ET.SubElement(
                    node,
                    "option",
                    {
                        "id": option.id,
                        "pl_prefix": option.pl_prefix,
                        "jp_prefix": option.jp_prefix,
                        "weight": str(option.weight),
                    },
                )
        _write_xml(root, path)

    def _save_patterns(self, path: Path) -> None:
        root = ET.Element("sentence_maps", {"schema_version": "6"})
        patterns = ET.SubElement(root, "sentence_patterns")
        for pattern in self.patterns.values():
            node = ET.SubElement(
                patterns,
                "sentence_pattern",
                {"id": pattern.id, "label": pattern.label},
            )
            ET.SubElement(node, "question").text = pattern.question
            ET.SubElement(node, "answer").text = pattern.answer
        _write_xml(root, path)

    def analyze_pattern(
        self,
        pattern: SentencePattern | str,
    ) -> PatternAnalysis:
        if isinstance(pattern, str):
            value = self.patterns.get(pattern)
            if value is None:
                raise ProjectError(f"Unknown sentence pattern: {pattern}")
            pattern = value
        expressions = TOKEN_PATTERN.findall(
            pattern.answer + "\n" + pattern.question
        )
        word_slots: List[str] = []
        entity_slots: List[str] = []
        specs_by_slot: Dict[str, List[str]] = {}
        for expression in expressions:
            root, field_name = _token_parts(expression)
            if root == "context":
                continue
            kind = self._project_slot_kind(root)
            if kind == "entity":
                if field_name and field_name not in ENTITY_FIELDS:
                    raise ProjectError(
                        f"Unknown entity field '{field_name}' in "
                        f"token '{{{expression}}}'."
                    )
                if root not in entity_slots:
                    entity_slots.append(root)
                continue
            if root not in word_slots:
                word_slots.append(root)
            if self._is_project_form_selector(root, field_name):
                specs_by_slot.setdefault(root, []).append(
                    field_name or self._project_default_form_spec(root)
                )

        primary_specs: Dict[str, str] = {}
        dynamic_slots: List[str] = []
        for slot_id in word_slots:
            specs = specs_by_slot.get(slot_id, [])
            if "form" in specs:
                primary = "form"
            elif specs:
                primary = specs[0]
            else:
                primary = self._project_default_form_spec(slot_id)
            primary_specs[slot_id] = primary
            if primary == "form":
                dynamic_slots.append(slot_id)
        if len(dynamic_slots) > 1:
            raise ProjectError(
                f"Sentence pattern '{pattern.id}' has more than one dynamic "
                f".form target: {', '.join(dynamic_slots)}."
            )

        driver_for_entity: Dict[str, str] = {}
        for entity_slot in entity_slots:
            relation_verbs: List[str] = []
            for slot_id in word_slots:
                if self._project_slot_kind(slot_id) != "verb":
                    continue
                filter_tag = _word_slot_filter_tag(slot_id)
                candidates = self.words.get("verbs", {}).values()
                if filter_tag:
                    candidates = (
                        word
                        for word in candidates
                        if filter_tag in word.categories
                    )
                if any(word.relations.get(entity_slot) for word in candidates):
                    relation_verbs.append(slot_id)

            dynamic_driver = (
                dynamic_slots[0]
                if dynamic_slots
                and dynamic_slots[0] in relation_verbs
                and self._project_slot_kind(dynamic_slots[0]) == "verb"
                else ""
            )
            if dynamic_driver:
                driver_for_entity[entity_slot] = dynamic_driver
            elif len(relation_verbs) == 1:
                driver_for_entity[entity_slot] = relation_verbs[0]
            elif len(relation_verbs) > 1:
                raise ProjectError(
                    f"Entity role '{entity_slot}' could be supplied by more "
                    f"than one verb slot in pattern '{pattern.id}': "
                    f"{', '.join(relation_verbs)}. Make the intended verb the "
                    "dynamic .form target or use distinct role names."
                )
        entity_for_adjective: Dict[str, str] = {}
        for slot_id in word_slots:
            if (
                self._project_slot_kind(slot_id) != "adjective"
                or not entity_slots
            ):
                continue
            if len(entity_slots) > 1:
                raise ProjectError(
                    f"Adjective slot '{slot_id}' has more than one possible "
                    f"entity in pattern '{pattern.id}': "
                    f"{', '.join(entity_slots)}."
                )
            entity_for_adjective[slot_id] = entity_slots[0]
        return PatternAnalysis(
            word_slots=word_slots,
            entity_slots=entity_slots,
            dynamic_slot=dynamic_slots[0] if dynamic_slots else "",
            primary_specs=primary_specs,
            driver_for_entity=driver_for_entity,
            entity_for_adjective=entity_for_adjective,
        )

    def eligible_form_names(
        self,
        pattern_id: str,
        pattern_override: Optional[SentencePattern] = None,
    ) -> List[str]:
        pattern = pattern_override or self._pattern(pattern_id)
        analysis = self.analyze_pattern(pattern)
        if not analysis.dynamic_slot:
            return []
        dictionary_id = self._dictionary_for_slot(analysis.dynamic_slot)
        candidates = self._base_word_candidates(
            analysis.dynamic_slot,
            analysis,
            dynamic_form="",
            check_form=False,
        )
        # `.form` currently represents the eight core finite quiz choices.
        # Future-ready derived forms stay available through explicit selectors
        # such as `.te`, `.potential`, or their stored form name.
        order = (
            ADJECTIVE_FORM_ORDER[:8]
            if dictionary_id == "adjectives"
            else VERB_FORM_ORDER[:8]
        )
        return [
            form_name
            for form_name in order
            if any(form_name in word.forms for word in candidates)
        ]

    def compatible_choices(
        self,
        pattern_id: str,
        form_name: Optional[str] = None,
        word_choices: Optional[Mapping[str, str]] = None,
        entity_choices: Optional[Mapping[str, str]] = None,
        adjective_type: str = "",
        pattern_override: Optional[SentencePattern] = None,
    ) -> CompatibleChoices:
        """Project valid menu values from any partial simulator selection."""
        pattern = pattern_override or self._pattern(pattern_id)
        analysis = self.analyze_pattern(pattern)
        requested_words = dict(word_choices or {})
        requested_entities = dict(entity_choices or {})
        self._validate_choice_slots(
            analysis,
            requested_words,
            requested_entities,
        )
        form_names = self._candidate_form_names(
            pattern,
            analysis,
            form_name,
        )
        compatible_forms: List[str] = []
        word_values: Dict[str, List[Word]] = {
            slot_id: [] for slot_id in analysis.word_slots
        }
        entity_values: Dict[str, List[Entity]] = {
            slot_id: [] for slot_id in analysis.entity_slots
        }

        for candidate_form in form_names:
            projected = self._compatible_choices_for_form(
                analysis,
                candidate_form,
                requested_words,
                requested_entities,
                adjective_type,
            )
            if projected is None:
                continue
            compatible_forms.append(candidate_form)
            projected_words, projected_entities = projected
            for slot_id, values in projected_words.items():
                known_ids = {
                    value.id for value in word_values[slot_id]
                }
                word_values[slot_id].extend(
                    value for value in values if value.id not in known_ids
                )
            for slot_id, values in projected_entities.items():
                known_ids = {
                    value.id for value in entity_values[slot_id]
                }
                entity_values[slot_id].extend(
                    value for value in values if value.id not in known_ids
                )

        if not compatible_forms:
            raise ProjectError(
                f"Sentence pattern '{pattern.id}' has no compatible choices "
                "for the current simulator selection."
            )

        for values in word_values.values():
            values.sort(key=lambda word: (word.kana, word.id))
        for values in entity_values.values():
            values.sort(key=lambda entity: (entity.kana, entity.id))
        return CompatibleChoices(
            form_names=compatible_forms,
            words=word_values,
            entities=entity_values,
        )

    def complete_choices(
        self,
        pattern_id: str,
        form_name: Optional[str] = None,
        word_choices: Optional[Mapping[str, str]] = None,
        entity_choices: Optional[Mapping[str, str]] = None,
        adjective_type: str = "",
        rng: Optional[random.Random] = None,
        pattern_override: Optional[SentencePattern] = None,
    ) -> CompletedChoices:
        """Randomly complete a partial selection without breaking constraints."""
        rng = rng or random.Random()
        pattern = pattern_override or self._pattern(pattern_id)
        analysis = self.analyze_pattern(pattern)
        requested_words = dict(word_choices or {})
        requested_entities = dict(entity_choices or {})
        self._validate_choice_slots(
            analysis,
            requested_words,
            requested_entities,
        )
        form_names = self._candidate_form_names(
            pattern,
            analysis,
            form_name,
        )
        rng.shuffle(form_names)

        for candidate_form in form_names:
            completed = self._complete_choices_for_form(
                analysis,
                candidate_form,
                requested_words,
                requested_entities,
                adjective_type,
                rng,
            )
            if completed is not None:
                return CompletedChoices(
                    form_name=candidate_form,
                    words=completed[0],
                    entities=completed[1],
                )

        raise ProjectError(
            f"Sentence pattern '{pattern.id}' has no compatible completion "
            "for the current simulator selection."
        )

    def build_preview(
        self,
        pattern_id: str,
        form_name: Optional[str] = None,
        word_choices: Optional[Mapping[str, str]] = None,
        entity_choices: Optional[Mapping[str, str]] = None,
        adjective_type: str = "",
        rng: Optional[random.Random] = None,
        pattern_override: Optional[SentencePattern] = None,
    ) -> Preview:
        rng = rng or random.Random()
        pattern = pattern_override or self._pattern(pattern_id)
        analysis = self.analyze_pattern(pattern)
        dynamic_form = form_name or ""
        if analysis.dynamic_slot and not dynamic_form:
            forms = self.eligible_form_names(
                pattern_id,
                pattern_override=pattern,
            )
            if not forms:
                raise ProjectError(
                    f"Sentence pattern '{pattern.id}' has no usable quiz forms."
                )
            dynamic_form = rng.choice(forms)
        requested_words = dict(word_choices or {})
        requested_entities = dict(entity_choices or {})

        selected_words: Dict[str, Word] = {}
        selected_forms: Dict[str, Form] = {}
        word_ids: Dict[str, str] = {}
        form_names: Dict[str, str] = {}

        leading_word_slots = [
            slot
            for slot in analysis.word_slots
            if self._project_slot_kind(slot) != "adjective"
        ]
        for slot_id in leading_word_slots:
            candidates = self._base_word_candidates(
                slot_id,
                analysis,
                dynamic_form,
                check_form=True,
            )
            word = self._choose_word(
                slot_id,
                candidates,
                requested_words.get(slot_id),
                rng,
            )
            form = _word_form(
                word,
                analysis.primary_specs[slot_id],
                dynamic_form,
                slot_id,
            )
            selected_words[slot_id] = word
            selected_forms[slot_id] = form
            word_ids[slot_id] = word.id
            form_names[slot_id] = form.name

        selected_entities: Dict[str, Entity] = {}
        entity_ids: Dict[str, str] = {}
        for slot_id in analysis.entity_slots:
            candidates = self._entity_candidates(
                slot_id,
                analysis,
                selected_words,
            )
            requested_id = requested_entities.get(slot_id)
            entity = self._choose_entity(slot_id, candidates, requested_id, rng)
            selected_entities[slot_id] = entity
            entity_ids[slot_id] = entity.id

        adjective_slots = [
            slot
            for slot in analysis.word_slots
            if self._project_slot_kind(slot) == "adjective"
        ]
        for slot_id in adjective_slots:
            candidates = self._adjective_candidates(
                slot_id,
                analysis,
                selected_entities,
                dynamic_form,
                adjective_type,
            )
            word = self._choose_word(
                slot_id,
                candidates,
                requested_words.get(slot_id),
                rng,
            )
            form = _word_form(
                word,
                analysis.primary_specs[slot_id],
                dynamic_form,
                slot_id,
            )
            selected_words[slot_id] = word
            selected_forms[slot_id] = form
            word_ids[slot_id] = word.id
            form_names[slot_id] = form.name

        context_id = "none"
        if analysis.dynamic_slot:
            context_id = selected_forms[analysis.dynamic_slot].context
        else:
            for slot_id in analysis.word_slots:
                form = selected_forms.get(slot_id)
                if form and form.context != "none":
                    context_id = form.context
        pool = self.contexts.get(context_id)
        if pool is None:
            raise ProjectError(
                f"Sentence pattern '{pattern.id}' resolved missing context "
                f"pool '{context_id}'."
            )
        context_option = pool.choose(rng)

        replacements = self._build_replacements(
            pattern,
            analysis,
            selected_words,
            selected_forms,
            selected_entities,
            dynamic_form,
            pool,
            context_option,
        )
        source = self._render(pattern.question, replacements)
        answer = self._render(pattern.answer, replacements)
        matches = self._compatibility_matches(
            analysis,
            selected_words,
            selected_entities,
        )
        return Preview(
            source=source,
            answer=answer,
            pattern_id=pattern.id,
            dynamic_slot=analysis.dynamic_slot,
            dynamic_form=dynamic_form,
            words=word_ids,
            forms=form_names,
            entities=entity_ids,
            context_pool=pool.id,
            context_option=context_option.id,
            replacements=replacements,
            matches=matches,
        )

    def _candidate_form_names(
        self,
        pattern: SentencePattern,
        analysis: PatternAnalysis,
        form_name: Optional[str],
    ) -> List[str]:
        if not analysis.dynamic_slot:
            return [""]
        eligible = self.eligible_form_names(
            pattern.id,
            pattern_override=pattern,
        )
        if form_name:
            if form_name not in eligible:
                raise ProjectError(
                    f"Form '{form_name}' is not available for sentence "
                    f"pattern '{pattern.id}'."
                )
            return [form_name]
        return list(eligible)

    @staticmethod
    def _validate_choice_slots(
        analysis: PatternAnalysis,
        word_choices: Mapping[str, str],
        entity_choices: Mapping[str, str],
    ) -> None:
        unknown_words = set(word_choices) - set(analysis.word_slots)
        unknown_entities = set(entity_choices) - set(
            analysis.entity_slots
        )
        if unknown_words or unknown_entities:
            unknown = sorted(unknown_words | unknown_entities)
            raise ProjectError(
                "Unknown simulator slot(s): " + ", ".join(unknown)
            )

    def _compatible_choices_for_form(
        self,
        analysis: PatternAnalysis,
        dynamic_form: str,
        requested_words: Mapping[str, str],
        requested_entities: Mapping[str, str],
        adjective_type: str,
    ) -> Optional[
        tuple[Dict[str, List[Word]], Dict[str, List[Entity]]]
    ]:
        words: Dict[str, List[Word]] = {}
        for slot_id in analysis.word_slots:
            if self._project_slot_kind(slot_id) == "adjective":
                values = self._adjective_candidates(
                    slot_id,
                    analysis,
                    {},
                    dynamic_form,
                    adjective_type,
                )
            else:
                values = self._base_word_candidates(
                    slot_id,
                    analysis,
                    dynamic_form,
                    check_form=True,
                )
            requested_id = requested_words.get(slot_id)
            if requested_id:
                values = [
                    value for value in values
                    if value.id == requested_id
                ]
            if not values:
                return None
            words[slot_id] = values

        entities: Dict[str, List[Entity]] = {}
        for slot_id in analysis.entity_slots:
            values = sorted(
                self.entities.values(),
                key=lambda entity: (entity.kana, entity.id),
            )
            requested_id = requested_entities.get(slot_id)
            if requested_id:
                values = [
                    value for value in values
                    if value.id == requested_id
                ]
            if not values:
                return None
            entities[slot_id] = values

        changed = True
        while changed:
            changed = False

            for entity_slot, driver_slot in (
                analysis.driver_for_entity.items()
            ):
                driver_values = words[driver_slot]
                entity_values = entities[entity_slot]

                def relation_matches(
                    word: Word,
                    entity: Entity,
                ) -> bool:
                    return _intersects(
                        word.relations.get(entity_slot, []),
                        self.effective_entity_tags(entity),
                    )

                filtered_words = [
                    word
                    for word in driver_values
                    if any(
                        relation_matches(word, entity)
                        for entity in entity_values
                    )
                ]
                filtered_entities = [
                    entity
                    for entity in entity_values
                    if any(
                        relation_matches(word, entity)
                        for word in filtered_words
                    )
                ]
                if not filtered_words or not filtered_entities:
                    return None
                if len(filtered_words) != len(driver_values):
                    words[driver_slot] = filtered_words
                    changed = True
                if len(filtered_entities) != len(entity_values):
                    entities[entity_slot] = filtered_entities
                    changed = True

            for adjective_slot, entity_slot in (
                analysis.entity_for_adjective.items()
            ):
                adjective_values = words[adjective_slot]
                entity_values = entities[entity_slot]

                def adjective_matches(
                    adjective: Word,
                    entity: Entity,
                ) -> bool:
                    return _intersects(
                        adjective.categories,
                        self.effective_entity_tags(entity),
                    )

                filtered_adjectives = [
                    adjective
                    for adjective in adjective_values
                    if any(
                        adjective_matches(adjective, entity)
                        for entity in entity_values
                    )
                ]
                filtered_entities = [
                    entity
                    for entity in entity_values
                    if any(
                        adjective_matches(adjective, entity)
                        for adjective in filtered_adjectives
                    )
                ]
                if not filtered_adjectives or not filtered_entities:
                    return None
                if len(filtered_adjectives) != len(adjective_values):
                    words[adjective_slot] = filtered_adjectives
                    changed = True
                if len(filtered_entities) != len(entity_values):
                    entities[entity_slot] = filtered_entities
                    changed = True

        return words, entities

    def _complete_choices_for_form(
        self,
        analysis: PatternAnalysis,
        dynamic_form: str,
        requested_words: Dict[str, str],
        requested_entities: Dict[str, str],
        adjective_type: str,
        rng: random.Random,
    ) -> Optional[tuple[Dict[str, str], Dict[str, str]]]:
        projected = self._compatible_choices_for_form(
            analysis,
            dynamic_form,
            requested_words,
            requested_entities,
            adjective_type,
        )
        if projected is None:
            return None

        word_values, entity_values = projected
        unassigned: List[tuple[str, str, List[str]]] = []
        for slot_id, values in word_values.items():
            if slot_id not in requested_words:
                unassigned.append(
                    ("word", slot_id, [value.id for value in values])
                )
        for slot_id, values in entity_values.items():
            if slot_id not in requested_entities:
                unassigned.append(
                    ("entity", slot_id, [value.id for value in values])
                )
        if not unassigned:
            return dict(requested_words), dict(requested_entities)

        kind, slot_id, values = min(
            unassigned,
            key=lambda item: len(item[2]),
        )
        rng.shuffle(values)
        for value in values:
            next_words = dict(requested_words)
            next_entities = dict(requested_entities)
            if kind == "word":
                next_words[slot_id] = value
            else:
                next_entities[slot_id] = value
            completed = self._complete_choices_for_form(
                analysis,
                dynamic_form,
                next_words,
                next_entities,
                adjective_type,
                rng,
            )
            if completed is not None:
                return completed
        return None

    def iter_possibilities(
        self,
        pattern_id: str,
        form_name: Optional[str] = None,
        word_choices: Optional[Mapping[str, str]] = None,
        entity_choices: Optional[Mapping[str, str]] = None,
        adjective_type: str = "",
        pattern_override: Optional[SentencePattern] = None,
    ) -> Iterator[Preview]:
        """Yield every valid sentence for one simulator configuration."""
        pattern = pattern_override or self._pattern(pattern_id)
        analysis = self.analyze_pattern(pattern)
        if analysis.dynamic_slot:
            form_names = (
                [form_name]
                if form_name
                else self.eligible_form_names(
                    pattern_id,
                    pattern_override=pattern,
                )
            )
        else:
            form_names = [""]
        if not form_names:
            raise ProjectError(
                f"Sentence pattern '{pattern.id}' has no usable quiz forms."
            )

        requested_words = dict(word_choices or {})
        requested_entities = dict(entity_choices or {})
        leading_word_slots = [
            slot
            for slot in analysis.word_slots
            if self._project_slot_kind(slot) != "adjective"
        ]
        adjective_slots = [
            slot
            for slot in analysis.word_slots
            if self._project_slot_kind(slot) == "adjective"
        ]

        for dynamic_form in form_names:
            leading_word_candidates: List[List[Word]] = []
            for slot_id in leading_word_slots:
                values = self._base_word_candidates(
                    slot_id,
                    analysis,
                    dynamic_form,
                    check_form=True,
                )
                leading_word_candidates.append(
                    self._requested_word_candidates(
                        slot_id,
                        values,
                        requested_words.get(slot_id),
                    )
                )

            for leading_word_values in product(*leading_word_candidates):
                selected_words = dict(
                    zip(leading_word_slots, leading_word_values)
                )
                selected_forms = {
                    slot_id: _word_form(
                        word,
                        analysis.primary_specs[slot_id],
                        dynamic_form,
                        slot_id,
                    )
                    for slot_id, word in selected_words.items()
                }

                entity_candidates: List[List[Entity]] = []
                for slot_id in analysis.entity_slots:
                    values = self._entity_candidates(
                        slot_id,
                        analysis,
                        selected_words,
                    )
                    entity_candidates.append(
                        self._requested_entity_candidates(
                            slot_id,
                            values,
                            requested_entities.get(slot_id),
                        )
                    )

                for entity_values in product(*entity_candidates):
                    selected_entities = dict(
                        zip(analysis.entity_slots, entity_values)
                    )
                    adjective_candidates: List[List[Word]] = []
                    for slot_id in adjective_slots:
                        values = self._adjective_candidates(
                            slot_id,
                            analysis,
                            selected_entities,
                            dynamic_form,
                            adjective_type,
                        )
                        adjective_candidates.append(
                            self._requested_word_candidates(
                                slot_id,
                                values,
                                requested_words.get(slot_id),
                            )
                        )

                    for adjective_values in product(*adjective_candidates):
                        words = dict(selected_words)
                        forms = dict(selected_forms)
                        for slot_id, word in zip(
                            adjective_slots,
                            adjective_values,
                        ):
                            words[slot_id] = word
                            forms[slot_id] = _word_form(
                                word,
                                analysis.primary_specs[slot_id],
                                dynamic_form,
                                slot_id,
                            )

                        pool = self._resolved_context_pool(
                            pattern,
                            analysis,
                            forms,
                        )
                        matches = self._compatibility_matches(
                            analysis,
                            words,
                            selected_entities,
                        )
                        for option in pool.options.values():
                            replacements = self._build_replacements(
                                pattern,
                                analysis,
                                words,
                                forms,
                                selected_entities,
                                dynamic_form,
                                pool,
                                option,
                            )
                            yield Preview(
                                source=self._render(
                                    pattern.question,
                                    replacements,
                                ),
                                answer=self._render(
                                    pattern.answer,
                                    replacements,
                                ),
                                pattern_id=pattern.id,
                                dynamic_slot=analysis.dynamic_slot,
                                dynamic_form=dynamic_form,
                                words={
                                    slot_id: word.id
                                    for slot_id, word in words.items()
                                },
                                forms={
                                    slot_id: form.name
                                    for slot_id, form in forms.items()
                                },
                                entities={
                                    slot_id: entity.id
                                    for slot_id, entity
                                    in selected_entities.items()
                                },
                                context_pool=pool.id,
                                context_option=option.id,
                                replacements=replacements,
                                matches=matches,
                            )

    def count_possibilities(
        self,
        pattern_id: str,
        form_name: Optional[str] = None,
        word_choices: Optional[Mapping[str, str]] = None,
        entity_choices: Optional[Mapping[str, str]] = None,
        adjective_type: str = "",
        pattern_override: Optional[SentencePattern] = None,
    ) -> PossibilityStats:
        """Calculate exact counts and compatibility metadata."""
        pattern = pattern_override or self._pattern(pattern_id)
        analysis = self.analyze_pattern(pattern)
        vocabulary_keys: set[tuple[object, ...]] = set()
        questions: set[str] = set()
        answers: set[str] = set()
        forms: set[str] = set()
        contexts: set[str] = set()
        word_ids: Dict[str, set[str]] = {
            slot_id: set() for slot_id in analysis.word_slots
        }
        entity_ids: Dict[str, set[str]] = {
            slot_id: set() for slot_id in analysis.entity_slots
        }
        connections: Dict[str, set[tuple[str, str]]] = {}
        generated = 0

        for preview in self.iter_possibilities(
            pattern_id,
            form_name=form_name,
            word_choices=word_choices,
            entity_choices=entity_choices,
            adjective_type=adjective_type,
            pattern_override=pattern,
        ):
            generated += 1
            questions.add(preview.source)
            answers.add(preview.answer)
            forms.add(preview.dynamic_form or "fixed")
            contexts.add(
                f"{preview.context_pool}/{preview.context_option}"
            )
            for slot_id, word_id in preview.words.items():
                word_ids[slot_id].add(word_id)
            for slot_id, entity_id in preview.entities.items():
                entity_ids[slot_id].add(entity_id)
            vocabulary_keys.add(
                (
                    preview.dynamic_form,
                    tuple(sorted(preview.words.items())),
                    tuple(sorted(preview.entities.items())),
                )
            )
            for entity_slot, driver_slot in (
                analysis.driver_for_entity.items()
            ):
                key = f"{driver_slot} → {entity_slot}"
                connections.setdefault(key, set()).add(
                    (
                        preview.words[driver_slot],
                        preview.entities[entity_slot],
                    )
                )
            for slot_id, entity_slot in (
                analysis.entity_for_adjective.items()
            ):
                key = f"{entity_slot} → {slot_id}"
                connections.setdefault(key, set()).add(
                    (
                        preview.entities[entity_slot],
                        preview.words[slot_id],
                    )
                )

        if not generated:
            raise ProjectError(
                f"Sentence pattern '{pattern.id}' has no possibilities "
                "for the selected filters."
            )
        return PossibilityStats(
            pattern_id=pattern.id,
            vocabulary_combinations=len(vocabulary_keys),
            generated_sentences=generated,
            unique_questions=len(questions),
            unique_answers=len(answers),
            form_names=sorted(forms),
            context_options=sorted(contexts),
            word_counts={
                slot_id: len(values)
                for slot_id, values in word_ids.items()
            },
            entity_counts={
                slot_id: len(values)
                for slot_id, values in entity_ids.items()
            },
            connection_counts={
                key: len(values)
                for key, values in connections.items()
            },
        )

    @staticmethod
    def _requested_word_candidates(
        slot_id: str,
        candidates: List[Word],
        requested_id: Optional[str],
    ) -> List[Word]:
        if not requested_id:
            return candidates
        values = [word for word in candidates if word.id == requested_id]
        if not values:
            raise ProjectError(
                f"Word '{requested_id}' is not eligible for slot "
                f"'{slot_id}'."
            )
        return values

    @staticmethod
    def _requested_entity_candidates(
        slot_id: str,
        candidates: List[Entity],
        requested_id: Optional[str],
    ) -> List[Entity]:
        if not requested_id:
            return candidates
        values = [
            entity for entity in candidates if entity.id == requested_id
        ]
        if not values:
            raise ProjectError(
                f"Entity '{requested_id}' is not eligible for slot "
                f"'{slot_id}'."
            )
        return values

    def _resolved_context_pool(
        self,
        pattern: SentencePattern,
        analysis: PatternAnalysis,
        forms: Mapping[str, Form],
    ) -> ContextPool:
        context_id = "none"
        if analysis.dynamic_slot:
            context_id = forms[analysis.dynamic_slot].context
        else:
            for slot_id in analysis.word_slots:
                form = forms.get(slot_id)
                if form and form.context != "none":
                    context_id = form.context
        pool = self.contexts.get(context_id)
        if pool is None:
            raise ProjectError(
                f"Sentence pattern '{pattern.id}' resolved missing context "
                f"pool '{context_id}'."
            )
        return pool

    def profile_tags(self, profile_id: str) -> List[str]:
        """Expand one XML profile, including inherited profiles."""
        return self._profile_tags(profile_id, [])

    def _profile_tags(
        self,
        profile_id: str,
        path: List[str],
    ) -> List[str]:
        profile = self.tag_profiles.get(profile_id)
        if profile is None:
            raise ProjectError(f"Unknown tag profile: {profile_id}")
        if profile_id in path:
            cycle = " -> ".join(path + [profile_id])
            raise ProjectError(f"Tag profile inheritance cycle: {cycle}")
        result: List[str] = []
        for included in profile.includes:
            for tag in self._profile_tags(
                included,
                path + [profile_id],
            ):
                if tag not in result:
                    result.append(tag)
        for tag in profile.tags:
            if tag not in result:
                result.append(tag)
        return result

    def effective_entity_tags(self, entity: Entity | str) -> List[str]:
        """Return explicit and profile-inherited tags for an entity."""
        if isinstance(entity, str):
            value = self.entities.get(entity)
            if value is None:
                raise ProjectError(f"Unknown entity: {entity}")
            entity = value
        result: List[str] = []
        for profile_id in entity.profiles:
            for tag in self.profile_tags(profile_id):
                if tag not in result:
                    result.append(tag)
        for tag in entity.categories:
            if tag not in result:
                result.append(tag)
        return result

    def _base_word_candidates(
        self,
        slot_id: str,
        analysis: PatternAnalysis,
        dynamic_form: str,
        check_form: bool,
    ) -> List[Word]:
        dictionary_id = self._dictionary_for_slot(slot_id)
        values = list(self.words.get(dictionary_id, {}).values())
        filter_tag = _word_slot_filter_tag(slot_id)
        if filter_tag:
            values = [
                word
                for word in values
                if filter_tag in word.categories
            ]
        required_roles = [
            role
            for role, driver in analysis.driver_for_entity.items()
            if driver == slot_id
        ]
        for role in required_roles:
            values = [
                word
                for word in values
                if word.relations.get(role)
                and any(
                    _intersects(
                        self.effective_entity_tags(entity),
                        word.relations[role],
                    )
                    for entity in self.entities.values()
                )
            ]
        if check_form:
            spec = analysis.primary_specs[slot_id]
            values = [
                word
                for word in values
                if _has_form(word, spec, dynamic_form)
            ]
        return sorted(values, key=lambda word: (word.kana, word.id))

    def _entity_candidates(
        self,
        slot_id: str,
        analysis: PatternAnalysis,
        selected_words: Mapping[str, Word],
    ) -> List[Entity]:
        values = list(self.entities.values())
        if slot_id in analysis.driver_for_entity:
            driver = selected_words[analysis.driver_for_entity[slot_id]]
            accepted = self._allowed_entity_categories(driver, slot_id)
            values = [
                entity
                for entity in values
                if _intersects(
                    self.effective_entity_tags(entity),
                    accepted,
                )
            ]
        elif slot_id in analysis.entity_for_adjective.values():
            values = [
                entity
                for entity in values
                if any(
                    _intersects(
                        self.effective_entity_tags(entity),
                        adjective.categories,
                    )
                    for adjective in self.words.get(
                        "adjectives",
                        {},
                    ).values()
                )
            ]
        return sorted(values, key=lambda entity: (entity.kana, entity.id))

    def _adjective_candidates(
        self,
        slot_id: str,
        analysis: PatternAnalysis,
        selected_entities: Mapping[str, Entity],
        dynamic_form: str,
        adjective_type: str = "",
    ) -> List[Word]:
        values = list(self.words.get("adjectives", {}).values())
        required_type = adjective_type
        if slot_id == "i_adjective":
            required_type = "i_adjective"
        elif slot_id == "na_adjective":
            required_type = "na_adjective"
        if required_type:
            if required_type not in {"i_adjective", "na_adjective"}:
                raise ProjectError(
                    f"Unknown adjective type filter: {required_type}"
                )
            values = [
                word for word in values if word.type == required_type
            ]
        described_entity = selected_entities.get(
            analysis.entity_for_adjective.get(slot_id, "")
        )
        if described_entity:
            values = [
                word
                for word in values
                if _intersects(
                    word.categories,
                    self.effective_entity_tags(described_entity),
                )
            ]
        spec = analysis.primary_specs[slot_id]
        values = [
            word
            for word in values
            if _has_form(word, spec, dynamic_form)
        ]
        return sorted(values, key=lambda word: (word.kana, word.id))

    def _project_slot_kind(self, root: str) -> str:
        kind = _slot_kind(root)
        if kind != "entity":
            return kind
        if root in self.words or f"{root}s" in self.words:
            return "word"
        return "entity"

    def _project_default_form_spec(self, root: str) -> str:
        kind = self._project_slot_kind(root)
        if kind == "adjective" and root != "predicate":
            return "attributive"
        return "form"

    def _is_project_form_selector(
        self,
        root: str,
        field_name: str,
    ) -> bool:
        kind = self._project_slot_kind(root)
        if not field_name:
            return kind != "entity"
        if kind == "entity":
            return False
        return field_name not in WORD_METADATA_FIELDS

    def _dictionary_for_slot(self, slot_id: str) -> str:
        if slot_id in self.words:
            return slot_id
        plural_id = f"{slot_id}s"
        if plural_id in self.words:
            return plural_id
        if self._project_slot_kind(slot_id) == "adjective":
            return "adjectives"
        if self._project_slot_kind(slot_id) == "verb":
            return "verbs"
        raise ProjectError(
            f"No dictionary is declared for word slot '{slot_id}'."
        )

    @staticmethod
    def _allowed_entity_categories(word: Word, role: str) -> set[str]:
        return set(word.relations.get(role, []))

    def _compatibility_matches(
        self,
        analysis: PatternAnalysis,
        words: Mapping[str, Word],
        entities: Mapping[str, Entity],
    ) -> List[str]:
        matches: List[str] = []
        for slot_id, entity in entities.items():
            if entity.profiles:
                matches.append(
                    f"{slot_id}={entity.id} expands XML profile(s): "
                    f"{', '.join(entity.profiles)}"
                )
        for entity_slot, driver_slot in analysis.driver_for_entity.items():
            driver = words.get(driver_slot)
            entity = entities.get(entity_slot)
            if not driver or not entity:
                continue
            shared = sorted(
                set(driver.relations.get(entity_slot, []))
                & set(self.effective_entity_tags(entity))
            )
            matches.append(
                f"{driver_slot}={driver.id} can use {entity_slot}="
                f"{entity.id} through shared tag(s): "
                f"{', '.join(shared) or '(none)'}"
            )
        for slot_id, entity_slot in analysis.entity_for_adjective.items():
            described_entity = entities.get(entity_slot)
            word = words.get(slot_id)
            if described_entity and word:
                shared = sorted(
                    set(word.categories)
                    & set(self.effective_entity_tags(described_entity))
                )
                matches.append(
                    f"{slot_id}={word.id} describes {described_entity.id} "
                    f"through shared tag(s): {', '.join(shared) or '(none)'}"
                )
        for slot_id, word in words.items():
            filter_tag = _word_slot_filter_tag(slot_id)
            if filter_tag:
                matches.append(
                    f"{slot_id}={word.id} matched word tag: {filter_tag}"
                )
        return matches

    @staticmethod
    def _choose_word(
        slot_id: str,
        candidates: List[Word],
        requested_id: Optional[str],
        rng: random.Random,
    ) -> Word:
        if requested_id:
            candidates = [
                word for word in candidates if word.id == requested_id
            ]
            if not candidates:
                raise ProjectError(
                    f"Word '{requested_id}' is not eligible for slot "
                    f"'{slot_id}'."
                )
        if not candidates:
            raise ProjectError(f"No eligible words for slot '{slot_id}'.")
        return rng.choice(candidates)

    @staticmethod
    def _choose_entity(
        slot_id: str,
        candidates: List[Entity],
        requested_id: Optional[str],
        rng: random.Random,
    ) -> Entity:
        if requested_id:
            candidates = [
                entity for entity in candidates if entity.id == requested_id
            ]
            if not candidates:
                raise ProjectError(
                    f"Entity '{requested_id}' is not eligible for slot "
                    f"'{slot_id}'."
                )
        if not candidates:
            raise ProjectError(f"No eligible entities for slot '{slot_id}'.")
        return rng.choice(candidates)

    def _build_replacements(
        self,
        pattern: SentencePattern,
        analysis: PatternAnalysis,
        words: Mapping[str, Word],
        forms: Mapping[str, Form],
        entities: Mapping[str, Entity],
        dynamic_form: str,
        pool: ContextPool,
        option: ContextOption,
    ) -> Dict[str, str]:
        replacements: Dict[str, str] = {}
        expressions = TOKEN_PATTERN.findall(
            pattern.question + "\n" + pattern.answer
        )
        for expression in expressions:
            token = "{" + expression + "}"
            if token in replacements:
                continue
            root, field_name = _token_parts(expression)
            if root == "context":
                context_values = {
                    "id": pool.id,
                    "option": option.id,
                    "pl_prefix": option.pl_prefix,
                    "jp_prefix": option.jp_prefix,
                }
                if field_name not in context_values:
                    raise ProjectError(f"Unknown context field '{field_name}'.")
                replacements[token] = context_values[field_name]
                continue
            kind = self._project_slot_kind(root)
            if kind == "entity":
                replacements[token] = self._entity_token(
                    entities[root],
                    field_name,
                )
                continue
            word = words[root]
            primary_form = forms[root]
            if not field_name:
                field_name = self._project_default_form_spec(root)
            if self._is_project_form_selector(root, field_name):
                replacements[token] = _word_form(
                    word,
                    field_name,
                    dynamic_form,
                    root,
                ).value
                continue
            gender = (
                entities[
                    analysis.entity_for_adjective[root]
                ].gender
                if kind == "adjective"
                and root in analysis.entity_for_adjective
                else (
                    next(iter(entities.values())).gender
                    if len(entities) == 1
                    else ""
                )
            )
            values = {
                "id": word.id,
                "kana": word.kana,
                "kanji": word.kanji,
                "translation": word.translation,
                "type": word.type,
                "ending": word.ending,
                "categories": ", ".join(word.categories),
                "tags": ", ".join(word.categories),
                "prompt": primary_form.prompt_for(gender),
                "style": primary_form.style,
                "form_name": primary_form.name,
            }
            replacements[token] = values[field_name]
        return replacements

    def _entity_token(self, entity: Entity, field_name: str) -> str:
        if not field_name:
            return entity.kana
        values = {
            "id": entity.id,
            "kana": entity.kana,
            "kanji": entity.kanji,
            "gender": entity.gender,
            "profiles": ", ".join(entity.profiles),
            "categories": ", ".join(self.effective_entity_tags(entity)),
            "tags": ", ".join(self.effective_entity_tags(entity)),
        }
        values.update(
            {
                f"pl_{name}": entity.polish.get(name, "")
                for name in POLISH_FIELDS
            }
        )
        return values[field_name]

    @staticmethod
    def _render(text: str, replacements: Mapping[str, str]) -> str:
        result = text
        for token in sorted(replacements, key=len, reverse=True):
            result = result.replace(token, replacements[token])
        unresolved = TOKEN_PATTERN.findall(result)
        if unresolved:
            raise ProjectError(
                "Unresolved tokens: "
                + ", ".join("{" + value + "}" for value in unresolved)
            )
        return re.sub(r"[ \t]*\n[ \t]*", " ", result).strip()

    def validate(self) -> List[str]:
        issues: List[str] = []
        if "none" not in self.contexts:
            issues.append("ERROR: Context pool 'none' is required.")
        for pool in self.contexts.values():
            if not pool.options:
                issues.append(f"ERROR: Context pool '{pool.id}' is empty.")
            for option in pool.options.values():
                if option.weight < 1:
                    issues.append(
                        f"ERROR: Context option '{pool.id}/{option.id}' has "
                        "an invalid weight."
                    )
        for dictionary_id, words in self.words.items():
            if not words:
                issues.append(f"ERROR: Dictionary '{dictionary_id}' is empty.")
            for word in words.values():
                if not re.fullmatch(r"[a-z0-9_]+", word.id):
                    issues.append(
                        f"ERROR: Word ID '{dictionary_id}/{word.id}' must use "
                        "lowercase romaji, digits, or underscores."
                    )
                if not word.kana:
                    issues.append(
                        f"ERROR: Word '{dictionary_id}/{word.id}' has no kana."
                    )
                if dictionary_id == "adjectives" and not word.categories:
                    issues.append(
                        f"WARNING: Word '{dictionary_id}/{word.id}' has no "
                        "shared tags."
                    )
                for role, tags in word.relations.items():
                    if not role or not tags:
                        issues.append(
                            f"ERROR: Word '{dictionary_id}/{word.id}' has an "
                            "empty relationship."
                        )
                for form in word.forms.values():
                    if not form.value:
                        issues.append(
                            f"ERROR: Form '{dictionary_id}/{word.id}/"
                            f"{form.name}' has no Japanese value."
                        )
                    if form.context not in self.contexts:
                        issues.append(
                            f"ERROR: Form '{dictionary_id}/{word.id}/"
                            f"{form.name}' references missing context "
                            f"'{form.context}'."
                        )
        for profile in self.tag_profiles.values():
            if not re.fullmatch(r"[a-z0-9_]+", profile.id):
                issues.append(
                    f"ERROR: Tag profile ID '{profile.id}' must use "
                    "lowercase letters, digits, or underscores."
                )
            for included in profile.includes:
                if included not in self.tag_profiles:
                    issues.append(
                        f"ERROR: Tag profile '{profile.id}' includes missing "
                        f"profile '{included}'."
                    )
            try:
                self.profile_tags(profile.id)
            except ProjectError as exception:
                issues.append(f"ERROR: {exception}")
        for entity in self.entities.values():
            if not entity.kana:
                issues.append(f"ERROR: Entity '{entity.id}' has no kana.")
            for profile_id in entity.profiles:
                if profile_id not in self.tag_profiles:
                    issues.append(
                        f"ERROR: Entity '{entity.id}' references missing tag "
                        f"profile '{profile_id}'."
                    )
            if not entity.profiles and not entity.categories:
                issues.append(
                    f"WARNING: Entity '{entity.id}' has no tag profile or "
                    "explicit tags."
                )
        for pattern in self.patterns.values():
            if not pattern.question or not pattern.answer:
                issues.append(
                    f"ERROR: Sentence pattern '{pattern.id}' needs both "
                    "question and answer text."
                )
                continue
            try:
                analysis = self.analyze_pattern(pattern)
                forms = self.eligible_form_names(pattern.id)
                if analysis.dynamic_slot and not forms:
                    issues.append(
                        f"ERROR: Sentence pattern '{pattern.id}' has no "
                        "eligible dynamic forms."
                    )
                    continue
                test_forms = forms or [None]
                for form_name in test_forms:
                    self.build_preview(
                        pattern.id,
                        form_name=form_name,
                        rng=random.Random(1),
                    )
            except ProjectError as exception:
                issues.append(
                    f"ERROR: Sentence pattern '{pattern.id}' cannot render: "
                    f"{exception}"
                )
        return issues

    def _pattern(self, pattern_id: str) -> SentencePattern:
        pattern = self.patterns.get(pattern_id)
        if pattern is None:
            raise ProjectError(f"Unknown sentence pattern: {pattern_id}")
        return pattern


# Older code used the name Template for the same object.
Template = SentencePattern


__all__ = [
    "ADJECTIVE_FORM_ORDER",
    "ContextOption",
    "ContextPool",
    "Entity",
    "Form",
    "POLISH_FIELDS",
    "PatternAnalysis",
    "PossibilityStats",
    "Preview",
    "ProjectError",
    "QuizProject",
    "SentencePattern",
    "TagProfile",
    "TOKEN_PATTERN",
    "Template",
    "VERB_FORM_ORDER",
    "Word",
    "pattern_slot_names",
]
