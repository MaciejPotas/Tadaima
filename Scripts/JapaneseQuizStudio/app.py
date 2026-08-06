"""Minimal desktop editor for Japanese Quiz Studio v6."""

from __future__ import annotations

import argparse
from pathlib import Path
import random
import re
import tkinter as tk
from tkinter import filedialog, messagebox, ttk
from typing import Callable, Dict, Iterable, List, Optional, TypeVar

from quiz_core import (
    ContextOption,
    ContextPool,
    Entity,
    Form,
    ProjectError,
    QuizProject,
    SentencePattern,
    TagProfile,
    Word,
    pattern_slot_names,
)


APP_NAME = "Japanese Quiz Studio v6"
RANDOM_VALUE = "<random>"
FIXED_VALUE = "<fixed forms>"
SORT_ITEM = TypeVar("SORT_ITEM")


def csv_values(text: str) -> List[str]:
    return [
        value.strip()
        for value in text.replace("\n", ",").split(",")
        if value.strip()
    ]


def parse_relations(text: str) -> Dict[str, List[str]]:
    """Parse one optional role relationship per line."""
    result: Dict[str, List[str]] = {}
    for line in text.splitlines():
        expression = line.strip()
        if not expression:
            continue
        if ":" not in expression:
            raise ProjectError(
                "Expected a relationship in the form role: tag, tag.\n"
                f"Received: {expression}"
            )
        role, tags_text = (
            value.strip() for value in expression.split(":", 1)
        )
        tags = csv_values(tags_text)
        if not role or not tags:
            raise ProjectError(f"Invalid relationship: {expression}")
        result[role] = tags
    return result


def format_relations(relations: Dict[str, List[str]]) -> str:
    return "\n".join(
        f"{role}: {', '.join(tags)}"
        for role, tags in relations.items()
    )


def sort_text_items(
    items: Iterable[SORT_ITEM],
    value_getter: Callable[[SORT_ITEM], object],
    reverse: bool = False,
) -> List[SORT_ITEM]:
    return sorted(
        items,
        key=lambda item: str(value_getter(item) or "").casefold(),
        reverse=reverse,
    )


def entry(
    parent: tk.Widget,
    row: int,
    label: str,
    variable: tk.StringVar,
    column: int = 0,
    width: int = 28,
) -> ttk.Entry:
    ttk.Label(parent, text=label).grid(
        row=row,
        column=column,
        sticky="w",
        padx=(0, 8),
        pady=4,
    )
    widget = ttk.Entry(parent, textvariable=variable, width=width)
    widget.grid(row=row, column=column + 1, sticky="ew", pady=4)
    return widget


def button_row(
    parent: tk.Widget,
    row: int,
    buttons: List[tuple[str, object]],
) -> None:
    frame = ttk.Frame(parent)
    frame.grid(
        row=row,
        column=0,
        columnspan=4,
        sticky="ew",
        pady=(10, 2),
    )
    for index, (label, command) in enumerate(buttons):
        ttk.Button(frame, text=label, command=command).grid(
            row=0,
            column=index,
            padx=(0, 8),
        )


class EditorTab(ttk.Frame):
    def __init__(self, parent: tk.Widget, app: "QuizStudio") -> None:
        super().__init__(parent, padding=14)
        self.app = app

    @property
    def project(self) -> QuizProject:
        return self.app.project

    def changed(self, message: str) -> None:
        self.app.mark_changed(message)

    def refresh(self) -> None:
        pass


class WordsTab(EditorTab):
    def __init__(self, parent: tk.Widget, app: "QuizStudio") -> None:
        super().__init__(parent, app)
        self.selected_key: Optional[tuple[str, str]] = None
        self.working_forms: Dict[str, Form] = {}
        self.sort_column = "kana"
        self.sort_reverse = False
        self.columns = (
            ("dictionary", "File", 85),
            ("id", "Stable ID", 130),
            ("kana", "Kana", 105),
            ("meaning", "Meaning", 155),
            ("type", "Type", 110),
            ("ending", "Ending", 75),
            ("categories", "Word tags", 200),
        )

        pane = ttk.Panedwindow(self, orient="horizontal")
        pane.pack(fill="both", expand=True)
        left = ttk.Frame(pane, padding=(0, 0, 12, 0))
        right = ttk.Frame(pane)
        pane.add(left, weight=2)
        pane.add(right, weight=3)

        ttk.Label(
            left,
            text="Dictionary words",
            style="Section.TLabel",
        ).pack(anchor="w", pady=(0, 8))
        tree_frame = ttk.Frame(left)
        tree_frame.pack(fill="both", expand=True)
        tree_frame.columnconfigure(0, weight=1)
        tree_frame.rowconfigure(0, weight=1)
        self.tree = ttk.Treeview(
            tree_frame,
            columns=tuple(column[0] for column in self.columns),
            show="headings",
        )
        for name, title, width in self.columns:
            self.tree.heading(
                name,
                text=title,
                command=lambda column=name: self._sort(column),
            )
            self.tree.column(name, width=width, anchor="w")
        vertical = ttk.Scrollbar(
            tree_frame,
            orient="vertical",
            command=self.tree.yview,
        )
        horizontal = ttk.Scrollbar(
            tree_frame,
            orient="horizontal",
            command=self.tree.xview,
        )
        self.tree.configure(
            yscrollcommand=vertical.set,
            xscrollcommand=horizontal.set,
        )
        self.tree.grid(row=0, column=0, sticky="nsew")
        vertical.grid(row=0, column=1, sticky="ns")
        horizontal.grid(row=1, column=0, sticky="ew")
        self.tree.bind("<<TreeviewSelect>>", self._select_word)
        self._update_headings()

        word_box = ttk.LabelFrame(right, text="Word", padding=12)
        word_box.pack(fill="x")
        word_box.columnconfigure(1, weight=1)
        word_box.columnconfigure(3, weight=1)
        self.dictionary_var = tk.StringVar()
        self.id_var = tk.StringVar()
        self.kana_var = tk.StringVar()
        self.kanji_var = tk.StringVar()
        self.translation_var = tk.StringVar()
        self.type_var = tk.StringVar()
        self.ending_var = tk.StringVar()
        self.categories_var = tk.StringVar()
        ttk.Label(word_box, text="Dictionary").grid(
            row=0,
            column=0,
            sticky="w",
            padx=(0, 8),
            pady=4,
        )
        self.dictionary_combo = ttk.Combobox(
            word_box,
            textvariable=self.dictionary_var,
            state="readonly",
            width=25,
        )
        self.dictionary_combo.grid(row=0, column=1, sticky="ew", pady=4)
        entry(word_box, 0, "Stable romaji ID", self.id_var, 2)
        entry(word_box, 1, "Kana", self.kana_var)
        entry(word_box, 1, "Kanji", self.kanji_var, 2)
        entry(word_box, 2, "Translation", self.translation_var)
        entry(word_box, 2, "Type", self.type_var, 2)
        entry(word_box, 3, "Word tags (CSV)", self.categories_var)
        entry(word_box, 3, "Ending", self.ending_var, 2)
        ttk.Label(word_box, text="Optional relationships").grid(
            row=4,
            column=0,
            sticky="nw",
            padx=(0, 8),
            pady=4,
        )
        self.relations_text = tk.Text(
            word_box,
            height=3,
            wrap="none",
            relief="solid",
            borderwidth=1,
        )
        self.relations_text.grid(
            row=4,
            column=1,
            columnspan=3,
            sticky="ew",
            pady=4,
        )
        ttk.Label(
            word_box,
            text=(
                "One per line, for example: object: edible. "
                "Relationships are possibilities, not requirements."
            ),
            style="Muted.TLabel",
        ).grid(
            row=5,
            column=1,
            columnspan=3,
            sticky="w",
            pady=(0, 4),
        )
        button_row(
            word_box,
            6,
            [
                ("New word", self.new_word),
                ("Save word", self.save_word),
                ("Delete", self.delete_word),
            ],
        )

        forms_box = ttk.LabelFrame(right, text="Stored forms", padding=12)
        forms_box.pack(fill="both", expand=True, pady=(12, 0))
        forms_box.columnconfigure(0, weight=1)
        forms_box.rowconfigure(0, weight=1)
        form_tree_frame = ttk.Frame(forms_box)
        form_tree_frame.grid(
            row=0,
            column=0,
            columnspan=4,
            sticky="nsew",
        )
        form_tree_frame.columnconfigure(0, weight=1)
        form_tree_frame.rowconfigure(0, weight=1)
        self.forms_tree = ttk.Treeview(
            form_tree_frame,
            columns=("name", "value", "style", "prompt", "context"),
            show="headings",
            height=8,
        )
        for name, title, width in (
            ("name", "Form", 170),
            ("value", "Japanese", 155),
            ("style", "Display description", 155),
            ("prompt", "Default prompt", 210),
            ("context", "Context", 90),
        ):
            self.forms_tree.heading(name, text=title)
            self.forms_tree.column(name, width=width, anchor="w")
        form_scroll = ttk.Scrollbar(
            form_tree_frame,
            orient="vertical",
            command=self.forms_tree.yview,
        )
        self.forms_tree.configure(yscrollcommand=form_scroll.set)
        self.forms_tree.grid(row=0, column=0, sticky="nsew")
        form_scroll.grid(row=0, column=1, sticky="ns")
        self.forms_tree.bind("<<TreeviewSelect>>", self._select_form)

        self.form_name_var = tk.StringVar()
        self.form_value_var = tk.StringVar()
        self.form_context_var = tk.StringVar(value="none")
        self.form_style_var = tk.StringVar()
        self.prompt_var = tk.StringVar()
        self.prompt_m_var = tk.StringVar()
        self.prompt_f_var = tk.StringVar()
        self.prompt_n_var = tk.StringVar()
        self.prompt_p_var = tk.StringVar()
        entry(forms_box, 1, "Form name", self.form_name_var)
        entry(forms_box, 1, "Japanese value", self.form_value_var, 2)
        entry(forms_box, 2, "Context", self.form_context_var)
        entry(
            forms_box,
            2,
            "Display description",
            self.form_style_var,
            2,
        )
        entry(forms_box, 3, "Default prompt", self.prompt_var)
        entry(forms_box, 3, "Prompt masculine", self.prompt_m_var, 2)
        entry(forms_box, 4, "Prompt feminine", self.prompt_f_var)
        entry(forms_box, 4, "Prompt neuter", self.prompt_n_var, 2)
        entry(forms_box, 5, "Prompt plural", self.prompt_p_var)
        button_row(
            forms_box,
            6,
            [
                ("New form", self.new_form),
                ("Save form", self.save_form),
                ("Delete form", self.delete_form),
            ],
        )

    def refresh(self) -> None:
        dictionaries = sorted(self.project.words)
        self.dictionary_combo["values"] = dictionaries
        if dictionaries and self.dictionary_var.get() not in dictionaries:
            self.dictionary_var.set(dictionaries[0])
        self.tree.delete(*self.tree.get_children())
        records = [
            (dictionary_id, word)
            for dictionary_id in dictionaries
            for word in self.project.words[dictionary_id].values()
        ]
        records = sort_text_items(
            records,
            self._sort_value,
            self.sort_reverse,
        )
        for dictionary_id, word in records:
            iid = f"{dictionary_id}::{word.id}"
            self.tree.insert(
                "",
                "end",
                iid=iid,
                values=(
                    dictionary_id,
                    word.id,
                    word.kana,
                    word.translation,
                    word.type,
                    word.ending,
                    ", ".join(word.categories),
                ),
            )
        self._update_headings()
        if self.selected_key:
            iid = f"{self.selected_key[0]}::{self.selected_key[1]}"
            if self.tree.exists(iid):
                self.tree.selection_set(iid)
                self.tree.see(iid)

    def _sort_value(self, record: tuple[str, Word]) -> object:
        dictionary_id, word = record
        return {
            "dictionary": dictionary_id,
            "id": word.id,
            "kana": word.kana,
            "meaning": word.translation,
            "type": word.type,
            "ending": word.ending,
            "categories": ", ".join(word.categories),
        }[self.sort_column]

    def _sort(self, column: str) -> None:
        if self.sort_column == column:
            self.sort_reverse = not self.sort_reverse
        else:
            self.sort_column = column
            self.sort_reverse = False
        self.refresh()

    def _update_headings(self) -> None:
        for name, title, _width in self.columns:
            marker = ""
            if name == self.sort_column:
                marker = " ▼" if self.sort_reverse else " ▲"
            self.tree.heading(name, text=title + marker)

    def _select_word(self, _event: object = None) -> None:
        selection = self.tree.selection()
        if not selection:
            return
        dictionary_id, word_id = selection[0].split("::", 1)
        word = self.project.words[dictionary_id][word_id]
        self.selected_key = (dictionary_id, word_id)
        self.dictionary_var.set(dictionary_id)
        self.id_var.set(word.id)
        self.kana_var.set(word.kana)
        self.kanji_var.set(word.kanji)
        self.translation_var.set(word.translation)
        self.type_var.set(word.type)
        self.ending_var.set(word.ending)
        self.categories_var.set(", ".join(word.categories))
        self.relations_text.delete("1.0", "end")
        self.relations_text.insert(
            "1.0",
            format_relations(word.relations),
        )
        self.working_forms = {
            name: Form(**vars(form)) for name, form in word.forms.items()
        }
        self._refresh_forms()
        self.new_form()

    def new_word(self) -> None:
        self.selected_key = None
        for variable in (
            self.id_var,
            self.kana_var,
            self.kanji_var,
            self.translation_var,
            self.type_var,
            self.ending_var,
            self.categories_var,
        ):
            variable.set("")
        self.working_forms = {}
        self.relations_text.delete("1.0", "end")
        self._refresh_forms()
        self.new_form()

    def save_word(self) -> None:
        try:
            dictionary_id = self.dictionary_var.get().strip()
            word_id = self.id_var.get().strip()
            kana = self.kana_var.get().strip()
            if dictionary_id not in self.project.words:
                raise ProjectError("Choose a dictionary.")
            if not re.fullmatch(r"[a-z0-9_]+", word_id):
                raise ProjectError(
                    "Stable IDs must use lowercase romaji, digits, or "
                    "underscores."
                )
            if not kana:
                raise ProjectError("Kana is required.")
            if self.selected_key and self.selected_key != (
                dictionary_id,
                word_id,
            ):
                old_dictionary, old_id = self.selected_key
                self.project.words[old_dictionary].pop(old_id, None)
            existing = self.project.words[dictionary_id].get(word_id)
            if existing and self.selected_key != (dictionary_id, word_id):
                raise ProjectError(
                    f"Word ID '{word_id}' already exists in '{dictionary_id}'."
                )
            self.project.words[dictionary_id][word_id] = Word(
                dictionary_id=dictionary_id,
                id=word_id,
                translation=self.translation_var.get().strip(),
                kana=kana,
                kanji=self.kanji_var.get().strip(),
                type=self.type_var.get().strip(),
                ending=self.ending_var.get().strip(),
                categories=csv_values(self.categories_var.get()),
                relations=parse_relations(
                    self.relations_text.get("1.0", "end")
                ),
                forms={
                    name: Form(**vars(form))
                    for name, form in self.working_forms.items()
                },
            )
            self.selected_key = (dictionary_id, word_id)
            self.changed(f"Saved word {word_id}.")
            self.app.refresh_all()
        except ProjectError as exception:
            messagebox.showerror(APP_NAME, str(exception), parent=self)

    def delete_word(self) -> None:
        if not self.selected_key:
            return
        dictionary_id, word_id = self.selected_key
        if messagebox.askyesno(
            APP_NAME,
            f"Delete word '{word_id}'?",
            parent=self,
        ):
            self.project.words[dictionary_id].pop(word_id, None)
            self.new_word()
            self.changed(f"Deleted word {word_id}.")
            self.app.refresh_all()

    def _refresh_forms(self) -> None:
        self.forms_tree.delete(*self.forms_tree.get_children())
        for form in self.working_forms.values():
            self.forms_tree.insert(
                "",
                "end",
                iid=form.name,
                values=(
                    form.name,
                    form.value,
                    form.style,
                    form.prompt,
                    form.context,
                ),
            )

    def _select_form(self, _event: object = None) -> None:
        selection = self.forms_tree.selection()
        if not selection:
            return
        form = self.working_forms[selection[0]]
        self.form_name_var.set(form.name)
        self.form_value_var.set(form.value)
        self.form_context_var.set(form.context)
        self.form_style_var.set(form.style)
        self.prompt_var.set(form.prompt)
        self.prompt_m_var.set(form.prompt_m)
        self.prompt_f_var.set(form.prompt_f)
        self.prompt_n_var.set(form.prompt_n)
        self.prompt_p_var.set(form.prompt_p)

    def new_form(self) -> None:
        self.forms_tree.selection_remove(*self.forms_tree.selection())
        self.form_name_var.set("")
        self.form_value_var.set("")
        self.form_context_var.set("none")
        self.form_style_var.set("")
        self.prompt_var.set("")
        self.prompt_m_var.set("")
        self.prompt_f_var.set("")
        self.prompt_n_var.set("")
        self.prompt_p_var.set("")

    def save_form(self) -> None:
        name = self.form_name_var.get().strip()
        value = self.form_value_var.get().strip()
        if not name or not value:
            messagebox.showerror(
                APP_NAME,
                "Form name and Japanese value are required.",
                parent=self,
            )
            return
        selection = self.forms_tree.selection()
        if selection and selection[0] != name:
            self.working_forms.pop(selection[0], None)
        self.working_forms[name] = Form(
            name=name,
            value=value,
            context=self.form_context_var.get().strip() or "none",
            style=self.form_style_var.get().strip(),
            prompt=self.prompt_var.get().strip(),
            prompt_m=self.prompt_m_var.get().strip(),
            prompt_f=self.prompt_f_var.get().strip(),
            prompt_n=self.prompt_n_var.get().strip(),
            prompt_p=self.prompt_p_var.get().strip(),
        )
        self._refresh_forms()
        self.forms_tree.selection_set(name)
        self._sync_forms()
        self.changed(
            f"Updated form {name}."
            if self.selected_key
            else f"Updated form {name}. Save the word to keep it."
        )

    def delete_form(self) -> None:
        selection = self.forms_tree.selection()
        if not selection:
            return
        self.working_forms.pop(selection[0], None)
        self._refresh_forms()
        self.new_form()
        self._sync_forms()
        self.changed("Removed form.")

    def _sync_forms(self) -> None:
        if not self.selected_key:
            return
        dictionary_id, word_id = self.selected_key
        word = self.project.words.get(dictionary_id, {}).get(word_id)
        if word:
            word.forms = {
                name: Form(**vars(form))
                for name, form in self.working_forms.items()
            }


class EntitiesTab(EditorTab):
    def __init__(self, parent: tk.Widget, app: "QuizStudio") -> None:
        super().__init__(parent, app)
        self.selected_id: Optional[str] = None
        self.sort_column = "kana"
        self.sort_reverse = False
        self.columns = (
            ("id", "ID", 115),
            ("kana", "Kana", 105),
            ("meaning", "Polish", 135),
            ("gender", "Gender", 85),
            ("profiles", "Profiles", 150),
            ("categories", "Effective tags", 260),
        )
        pane = ttk.Panedwindow(self, orient="horizontal")
        pane.pack(fill="both", expand=True)
        left = ttk.Frame(pane, padding=(0, 0, 12, 0))
        right = ttk.Frame(pane)
        pane.add(left, weight=2)
        pane.add(right, weight=3)
        ttk.Label(
            left,
            text="Reusable entities",
            style="Section.TLabel",
        ).pack(anchor="w", pady=(0, 8))
        tree_frame = ttk.Frame(left)
        tree_frame.pack(fill="both", expand=True)
        tree_frame.columnconfigure(0, weight=1)
        tree_frame.rowconfigure(0, weight=1)
        self.tree = ttk.Treeview(
            tree_frame,
            columns=tuple(column[0] for column in self.columns),
            show="headings",
        )
        for name, title, width in self.columns:
            self.tree.heading(
                name,
                text=title,
                command=lambda column=name: self._sort(column),
            )
            self.tree.column(name, width=width, anchor="w")
        vertical = ttk.Scrollbar(
            tree_frame,
            orient="vertical",
            command=self.tree.yview,
        )
        horizontal = ttk.Scrollbar(
            tree_frame,
            orient="horizontal",
            command=self.tree.xview,
        )
        self.tree.configure(
            yscrollcommand=vertical.set,
            xscrollcommand=horizontal.set,
        )
        self.tree.grid(row=0, column=0, sticky="nsew")
        vertical.grid(row=0, column=1, sticky="ns")
        horizontal.grid(row=1, column=0, sticky="ew")
        self.tree.bind("<<TreeviewSelect>>", self._select)
        self._update_headings()

        box = ttk.LabelFrame(right, text="Entity", padding=12)
        box.pack(fill="x")
        box.columnconfigure(1, weight=1)
        box.columnconfigure(3, weight=1)
        self.vars = {
            name: tk.StringVar()
            for name in (
                "id",
                "kana",
                "kanji",
                "gender",
                "profiles",
                "categories",
                "nominative",
                "accusative",
                "genitive",
                "locative",
                "instrumental",
                "demonstrative",
            )
        }
        entry(box, 0, "Stable ID", self.vars["id"])
        entry(box, 0, "Gender", self.vars["gender"], 2)
        entry(box, 1, "Kana", self.vars["kana"])
        entry(box, 1, "Kanji", self.vars["kanji"], 2)
        entry(box, 2, "Tag profiles (CSV)", self.vars["profiles"])
        entry(box, 3, "Extra tags (CSV)", self.vars["categories"])
        entry(box, 4, "Polish nominative", self.vars["nominative"])
        entry(box, 4, "Accusative", self.vars["accusative"], 2)
        entry(box, 5, "Genitive", self.vars["genitive"])
        entry(box, 5, "Locative", self.vars["locative"], 2)
        entry(box, 6, "Instrumental", self.vars["instrumental"])
        entry(box, 6, "Demonstrative", self.vars["demonstrative"], 2)
        button_row(
            box,
            7,
            [
                ("New entity", self.new),
                ("Save entity", self.save),
                ("Delete", self.delete),
            ],
        )

        help_box = ttk.LabelFrame(
            right,
            text="Automatic reuse",
            padding=12,
        )
        help_box.pack(fill="x", pady=(12, 0))
        ttk.Label(
            help_box,
            text=(
                "Profiles inherit reusable tags from XML. A motorcycle can "
                "use only the vehicle profile and automatically receive its "
                "verb capabilities and adjective dimensions. Extra tags are "
                "only for exceptions."
            ),
            style="Muted.TLabel",
            wraplength=650,
            justify="left",
        ).pack(anchor="w")

    def refresh(self) -> None:
        self.tree.delete(*self.tree.get_children())
        values = sort_text_items(
            self.project.entities.values(),
            self._sort_value,
            self.sort_reverse,
        )
        for entity in values:
            self.tree.insert(
                "",
                "end",
                iid=entity.id,
                values=(
                    entity.id,
                    entity.kana,
                    entity.polish.get("nominative", ""),
                    entity.gender,
                    ", ".join(entity.profiles),
                    ", ".join(
                        self.project.effective_entity_tags(entity)
                    ),
                ),
            )
        self._update_headings()
        if self.selected_id and self.tree.exists(self.selected_id):
            self.tree.selection_set(self.selected_id)
            self.tree.see(self.selected_id)

    def _sort_value(self, entity: Entity) -> object:
        return {
            "id": entity.id,
            "kana": entity.kana,
            "meaning": entity.polish.get("nominative", ""),
            "gender": entity.gender,
            "profiles": ", ".join(entity.profiles),
            "categories": ", ".join(
                self.project.effective_entity_tags(entity)
            ),
        }[self.sort_column]

    def _sort(self, column: str) -> None:
        if self.sort_column == column:
            self.sort_reverse = not self.sort_reverse
        else:
            self.sort_column = column
            self.sort_reverse = False
        self.refresh()

    def _update_headings(self) -> None:
        for name, title, _width in self.columns:
            marker = ""
            if name == self.sort_column:
                marker = " ▼" if self.sort_reverse else " ▲"
            self.tree.heading(name, text=title + marker)

    def _select(self, _event: object = None) -> None:
        selection = self.tree.selection()
        if not selection:
            return
        entity = self.project.entities[selection[0]]
        self.selected_id = entity.id
        self.vars["id"].set(entity.id)
        self.vars["kana"].set(entity.kana)
        self.vars["kanji"].set(entity.kanji)
        self.vars["gender"].set(entity.gender)
        self.vars["profiles"].set(", ".join(entity.profiles))
        self.vars["categories"].set(", ".join(entity.categories))
        for name in (
            "nominative",
            "accusative",
            "genitive",
            "locative",
            "instrumental",
            "demonstrative",
        ):
            self.vars[name].set(entity.polish.get(name, ""))

    def new(self) -> None:
        self.selected_id = None
        for variable in self.vars.values():
            variable.set("")

    def save(self) -> None:
        entity_id = self.vars["id"].get().strip()
        kana = self.vars["kana"].get().strip()
        if not entity_id or not kana:
            messagebox.showerror(
                APP_NAME,
                "Stable ID and kana are required.",
                parent=self,
            )
            return
        profiles = csv_values(self.vars["profiles"].get())
        unknown_profiles = [
            value
            for value in profiles
            if value not in self.project.tag_profiles
        ]
        if unknown_profiles:
            messagebox.showerror(
                APP_NAME,
                "Unknown tag profile(s): "
                + ", ".join(unknown_profiles),
                parent=self,
            )
            return
        if self.selected_id and self.selected_id != entity_id:
            self.project.entities.pop(self.selected_id, None)
        self.project.entities[entity_id] = Entity(
            id=entity_id,
            kana=kana,
            kanji=self.vars["kanji"].get().strip(),
            gender=self.vars["gender"].get().strip(),
            profiles=profiles,
            categories=csv_values(self.vars["categories"].get()),
            polish={
                name: self.vars[name].get().strip()
                for name in (
                    "nominative",
                    "accusative",
                    "genitive",
                    "locative",
                    "instrumental",
                    "demonstrative",
                )
            },
        )
        self.selected_id = entity_id
        self.changed(f"Saved entity {entity_id}.")
        self.app.refresh_all()

    def delete(self) -> None:
        if self.selected_id and messagebox.askyesno(
            APP_NAME,
            f"Delete entity '{self.selected_id}'?",
            parent=self,
        ):
            entity_id = self.selected_id
            self.project.entities.pop(entity_id, None)
            self.new()
            self.changed(f"Deleted entity {entity_id}.")
            self.app.refresh_all()


class TagProfilesTab(EditorTab):
    def __init__(self, parent: tk.Widget, app: "QuizStudio") -> None:
        super().__init__(parent, app)
        self.selected_id: Optional[str] = None
        pane = ttk.Panedwindow(self, orient="horizontal")
        pane.pack(fill="both", expand=True)
        left = ttk.Frame(pane, padding=(0, 0, 12, 0))
        right = ttk.Frame(pane)
        pane.add(left, weight=2)
        pane.add(right, weight=3)

        ttk.Label(
            left,
            text="Reusable tag profiles",
            style="Section.TLabel",
        ).pack(anchor="w", pady=(0, 8))
        tree_frame = ttk.Frame(left)
        tree_frame.pack(fill="both", expand=True)
        tree_frame.columnconfigure(0, weight=1)
        tree_frame.rowconfigure(0, weight=1)
        self.tree = ttk.Treeview(
            tree_frame,
            columns=("id", "includes", "tags"),
            show="headings",
        )
        for name, title, width in (
            ("id", "Profile", 160),
            ("includes", "Includes", 190),
            ("tags", "Effective tags", 330),
        ):
            self.tree.heading(name, text=title)
            self.tree.column(name, width=width, anchor="w")
        scrollbar = ttk.Scrollbar(
            tree_frame,
            orient="vertical",
            command=self.tree.yview,
        )
        self.tree.configure(yscrollcommand=scrollbar.set)
        self.tree.grid(row=0, column=0, sticky="nsew")
        scrollbar.grid(row=0, column=1, sticky="ns")
        self.tree.bind("<<TreeviewSelect>>", self._select)

        box = ttk.LabelFrame(right, text="Tag profile", padding=12)
        box.pack(fill="both", expand=True)
        box.columnconfigure(1, weight=1)
        box.rowconfigure(2, weight=1)
        self.id_var = tk.StringVar()
        self.includes_var = tk.StringVar()
        entry(box, 0, "Profile ID", self.id_var)
        entry(box, 1, "Included profiles (CSV)", self.includes_var)
        ttk.Label(box, text="Own tags, CSV or lines").grid(
            row=2,
            column=0,
            sticky="nw",
            padx=(0, 8),
            pady=4,
        )
        self.tags_text = tk.Text(
            box,
            height=14,
            wrap="word",
            relief="solid",
            borderwidth=1,
        )
        self.tags_text.grid(row=2, column=1, sticky="nsew", pady=4)
        button_row(
            box,
            3,
            [
                ("New profile", self.new),
                ("Save profile", self.save),
                ("Delete", self.delete),
            ],
        )
        ttk.Label(
            box,
            text=(
                "Profiles and inheritance are stored in sentence_vocabulary."
                "xml. Python only expands the XML references. It does not "
                "know what vehicle, food, text, or any tag means."
            ),
            style="Muted.TLabel",
            wraplength=650,
            justify="left",
        ).grid(
            row=4,
            column=0,
            columnspan=2,
            sticky="w",
            pady=(8, 0),
        )

    def refresh(self) -> None:
        self.tree.delete(*self.tree.get_children())
        for profile in self.project.tag_profiles.values():
            try:
                effective = self.project.profile_tags(profile.id)
            except ProjectError:
                effective = ["invalid"]
            self.tree.insert(
                "",
                "end",
                iid=profile.id,
                values=(
                    profile.id,
                    ", ".join(profile.includes),
                    ", ".join(effective),
                ),
            )
        if self.selected_id and self.tree.exists(self.selected_id):
            self.tree.selection_set(self.selected_id)
            self.tree.see(self.selected_id)

    def _select(self, _event: object = None) -> None:
        selection = self.tree.selection()
        if not selection:
            return
        profile = self.project.tag_profiles[selection[0]]
        self.selected_id = profile.id
        self.id_var.set(profile.id)
        self.includes_var.set(", ".join(profile.includes))
        self.tags_text.delete("1.0", "end")
        self.tags_text.insert("1.0", ", ".join(profile.tags))

    def new(self) -> None:
        self.selected_id = None
        self.id_var.set("")
        self.includes_var.set("")
        self.tags_text.delete("1.0", "end")

    def save(self) -> None:
        profile_id = self.id_var.get().strip()
        if not re.fullmatch(r"[a-z0-9_]+", profile_id):
            messagebox.showerror(
                APP_NAME,
                "Profile IDs must use lowercase letters, digits, or "
                "underscores.",
                parent=self,
            )
            return
        includes = csv_values(self.includes_var.get())
        unknown = [
            value
            for value in includes
            if value not in self.project.tag_profiles
            and value != self.selected_id
        ]
        if unknown:
            messagebox.showerror(
                APP_NAME,
                "Unknown included profile(s): " + ", ".join(unknown),
                parent=self,
            )
            return
        if (
            profile_id in self.project.tag_profiles
            and profile_id != self.selected_id
        ):
            messagebox.showerror(
                APP_NAME,
                f"Tag profile '{profile_id}' already exists.",
                parent=self,
            )
            return

        old_id = self.selected_id
        profile_snapshot = dict(self.project.tag_profiles)
        include_snapshot = {
            value.id: list(value.includes)
            for value in self.project.tag_profiles.values()
        }
        entity_snapshot = {
            value.id: list(value.profiles)
            for value in self.project.entities.values()
        }
        if old_id and old_id != profile_id:
            self.project.tag_profiles.pop(old_id, None)
            includes = [
                profile_id if value == old_id else value
                for value in includes
            ]
            for profile in self.project.tag_profiles.values():
                profile.includes = [
                    profile_id if value == old_id else value
                    for value in profile.includes
                ]
            for entity in self.project.entities.values():
                entity.profiles = [
                    profile_id if value == old_id else value
                    for value in entity.profiles
                ]

        candidate = TagProfile(
            id=profile_id,
            includes=includes,
            tags=csv_values(self.tags_text.get("1.0", "end")),
        )
        self.project.tag_profiles[profile_id] = candidate
        try:
            for value in self.project.tag_profiles:
                self.project.profile_tags(value)
        except ProjectError as exception:
            self.project.tag_profiles = profile_snapshot
            for value in self.project.tag_profiles.values():
                value.includes = include_snapshot[value.id]
            for value in self.project.entities.values():
                value.profiles = entity_snapshot[value.id]
            messagebox.showerror(APP_NAME, str(exception), parent=self)
            return
        self.selected_id = profile_id
        self.changed(f"Saved tag profile {profile_id}.")
        self.app.refresh_all()

    def delete(self) -> None:
        if not self.selected_id:
            return
        referenced_by = [
            f"entity {entity.id}"
            for entity in self.project.entities.values()
            if self.selected_id in entity.profiles
        ]
        referenced_by.extend(
            f"profile {profile.id}"
            for profile in self.project.tag_profiles.values()
            if self.selected_id in profile.includes
        )
        if referenced_by:
            messagebox.showerror(
                APP_NAME,
                "This profile is still used by:\n"
                + "\n".join(referenced_by[:15]),
                parent=self,
            )
            return
        if messagebox.askyesno(
            APP_NAME,
            f"Delete tag profile '{self.selected_id}'?",
            parent=self,
        ):
            profile_id = self.selected_id
            self.project.tag_profiles.pop(profile_id, None)
            self.new()
            self.changed(f"Deleted tag profile {profile_id}.")
            self.app.refresh_all()


class SentenceVocabularyTab(EditorTab):
    def __init__(self, parent: tk.Widget, app: "QuizStudio") -> None:
        super().__init__(parent, app)
        notebook = ttk.Notebook(self)
        notebook.pack(fill="both", expand=True)
        self.entities = EntitiesTab(notebook, app)
        self.profiles = TagProfilesTab(notebook, app)
        notebook.add(self.entities, text="Entities")
        notebook.add(self.profiles, text="Tag profiles")

    def refresh(self) -> None:
        self.entities.refresh()
        self.profiles.refresh()


class PatternsTab(EditorTab):
    def __init__(self, parent: tk.Widget, app: "QuizStudio") -> None:
        super().__init__(parent, app)
        self.selected_id: Optional[str] = None
        self.preview_seed = 1
        self.preview_job: Optional[str] = None
        pane = ttk.Panedwindow(self, orient="horizontal")
        pane.pack(fill="both", expand=True)
        left = ttk.Frame(pane, padding=(0, 0, 12, 0))
        right = ttk.Frame(pane)
        pane.add(left, weight=2)
        pane.add(right, weight=3)

        ttk.Label(
            left,
            text="Sentence patterns",
            style="Section.TLabel",
        ).pack(anchor="w", pady=(0, 8))
        tree_frame = ttk.Frame(left)
        tree_frame.pack(fill="both", expand=True)
        tree_frame.columnconfigure(0, weight=1)
        tree_frame.rowconfigure(0, weight=1)
        self.tree = ttk.Treeview(
            tree_frame,
            columns=("id", "label", "slots", "target"),
            show="headings",
        )
        for name, title, width in (
            ("id", "ID", 190),
            ("label", "Pattern", 210),
            ("slots", "Detected slots", 180),
            ("target", "Quiz target", 120),
        ):
            self.tree.heading(name, text=title)
            self.tree.column(name, width=width, anchor="w")
        scrollbar = ttk.Scrollbar(
            tree_frame,
            orient="vertical",
            command=self.tree.yview,
        )
        self.tree.configure(yscrollcommand=scrollbar.set)
        self.tree.grid(row=0, column=0, sticky="nsew")
        scrollbar.grid(row=0, column=1, sticky="ns")
        self.tree.bind("<<TreeviewSelect>>", self._select)

        box = ttk.LabelFrame(right, text="Sentence pattern", padding=12)
        box.pack(fill="both", expand=True, pady=(0, 10))
        box.columnconfigure(1, weight=1)
        box.rowconfigure(3, weight=1)
        box.rowconfigure(4, weight=1)
        self.id_var = tk.StringVar()
        self.label_var = tk.StringVar()
        self.slots_var = tk.StringVar(value="None")
        entry(box, 0, "Pattern ID", self.id_var)
        entry(box, 1, "Label", self.label_var)
        ttk.Label(box, text="Detected slots").grid(
            row=2,
            column=0,
            sticky="w",
            padx=(0, 8),
            pady=4,
        )
        ttk.Label(
            box,
            textvariable=self.slots_var,
            style="Muted.TLabel",
        ).grid(row=2, column=1, sticky="w", pady=4)
        ttk.Label(box, text="Polish question").grid(
            row=3,
            column=0,
            sticky="nw",
            padx=(0, 8),
            pady=4,
        )
        self.question_text = tk.Text(
            box,
            height=5,
            wrap="word",
            relief="solid",
            borderwidth=1,
        )
        self.question_text.grid(row=3, column=1, sticky="nsew", pady=4)
        ttk.Label(box, text="Japanese answer").grid(
            row=4,
            column=0,
            sticky="nw",
            padx=(0, 8),
            pady=4,
        )
        self.answer_text = tk.Text(
            box,
            height=5,
            wrap="word",
            relief="solid",
            borderwidth=1,
        )
        self.answer_text.grid(row=4, column=1, sticky="nsew", pady=4)
        button_row(
            box,
            5,
            [
                ("New pattern", self.new),
                ("Save pattern", self.save),
                ("Delete", self.delete),
            ],
        )
        ttk.Label(
            box,
            text=(
                "Dynamic target: {verb.form}, {adjective.form}, or "
                "{copula.form}\n"
                "Fixed grammar: {adjective.attributive}, {verb.te}, "
                "{verb_before.dictionary}"
            ),
            style="Muted.TLabel",
            justify="left",
        ).grid(
            row=6,
            column=0,
            columnspan=2,
            sticky="w",
            pady=(8, 0),
        )

        preview = ttk.LabelFrame(right, text="Live example", padding=12)
        preview.pack(fill="x")
        preview.columnconfigure(1, weight=1)
        self.preview_form_var = tk.StringVar(value=RANDOM_VALUE)
        ttk.Label(preview, text="Quiz form").grid(
            row=0,
            column=0,
            sticky="w",
            padx=(0, 8),
        )
        self.preview_form_combo = ttk.Combobox(
            preview,
            textvariable=self.preview_form_var,
            state="readonly",
        )
        self.preview_form_combo.grid(row=0, column=1, sticky="ew")
        self.preview_form_combo.bind(
            "<<ComboboxSelected>>",
            self._schedule_preview,
        )
        ttk.Button(
            preview,
            text="Randomize",
            command=self.randomize,
        ).grid(row=0, column=2, padx=(8, 0))
        self.preview_text = tk.Text(
            preview,
            height=12,
            wrap="word",
            relief="flat",
            background="#f7f8fa",
        )
        self.preview_text.grid(
            row=1,
            column=0,
            columnspan=3,
            sticky="ew",
            pady=(10, 0),
        )
        for widget in (self.question_text, self.answer_text):
            widget.bind("<KeyRelease>", self._schedule_preview)

    def refresh(self) -> None:
        self.tree.delete(*self.tree.get_children())
        for pattern in self.project.patterns.values():
            try:
                target = self.project.analyze_pattern(pattern).dynamic_slot
            except ProjectError:
                target = "invalid"
            self.tree.insert(
                "",
                "end",
                iid=pattern.id,
                values=(
                    pattern.id,
                    pattern.label,
                    ", ".join(pattern.slot_names()),
                    target or "fixed",
                ),
            )
        if self.selected_id and self.tree.exists(self.selected_id):
            self.tree.selection_set(self.selected_id)
        self._schedule_preview()

    def _select(self, _event: object = None) -> None:
        selection = self.tree.selection()
        if not selection:
            return
        pattern = self.project.patterns[selection[0]]
        self.selected_id = pattern.id
        self.id_var.set(pattern.id)
        self.label_var.set(pattern.label)
        self.question_text.delete("1.0", "end")
        self.question_text.insert("1.0", pattern.question)
        self.answer_text.delete("1.0", "end")
        self.answer_text.insert("1.0", pattern.answer)
        self._schedule_preview()

    def new(self) -> None:
        self.selected_id = None
        self.id_var.set("")
        self.label_var.set("")
        self.question_text.delete("1.0", "end")
        self.answer_text.delete("1.0", "end")
        self.slots_var.set("None")
        self._schedule_preview()

    def save(self) -> None:
        pattern_id = self.id_var.get().strip()
        question = self.question_text.get("1.0", "end").strip()
        answer = self.answer_text.get("1.0", "end").strip()
        if not pattern_id or not question or not answer:
            messagebox.showerror(
                APP_NAME,
                "Pattern ID, question, and answer are required.",
                parent=self,
            )
            return
        candidate = SentencePattern(
            id=pattern_id,
            label=self.label_var.get().strip() or pattern_id,
            question=question,
            answer=answer,
        )
        try:
            self.project.analyze_pattern(candidate)
            forms = self.project.eligible_form_names(
                pattern_id,
                pattern_override=candidate,
            )
            form_name = forms[0] if forms else None
            self.project.build_preview(
                pattern_id,
                form_name=form_name,
                pattern_override=candidate,
                rng=random.Random(1),
            )
        except ProjectError as exception:
            messagebox.showerror(APP_NAME, str(exception), parent=self)
            return
        if self.selected_id and self.selected_id != pattern_id:
            self.project.patterns.pop(self.selected_id, None)
        self.project.patterns[pattern_id] = candidate
        self.selected_id = pattern_id
        self.changed(f"Saved sentence pattern {pattern_id}.")
        self.app.refresh_all()

    def delete(self) -> None:
        if self.selected_id and messagebox.askyesno(
            APP_NAME,
            f"Delete sentence pattern '{self.selected_id}'?",
            parent=self,
        ):
            pattern_id = self.selected_id
            self.project.patterns.pop(pattern_id, None)
            self.new()
            self.changed(f"Deleted sentence pattern {pattern_id}.")
            self.app.refresh_all()

    def randomize(self) -> None:
        self.preview_seed += 1
        self._refresh_preview()

    def _schedule_preview(self, *_args: object) -> None:
        if self.preview_job:
            self.after_cancel(self.preview_job)
        self.preview_job = self.after(250, self._refresh_preview)

    def _editing_pattern(self) -> Optional[SentencePattern]:
        question = self.question_text.get("1.0", "end").strip()
        answer = self.answer_text.get("1.0", "end").strip()
        if not question or not answer:
            return None
        pattern_id = self.id_var.get().strip() or "unsaved_pattern"
        return SentencePattern(
            id=pattern_id,
            label=self.label_var.get().strip() or pattern_id,
            question=question,
            answer=answer,
        )

    def _refresh_preview(self) -> None:
        self.preview_job = None
        self.preview_text.delete("1.0", "end")
        pattern = self._editing_pattern()
        if not pattern:
            self.slots_var.set("None")
            self.preview_text.insert(
                "1.0",
                "Write a question and answer to see an example.",
            )
            return
        self.slots_var.set(", ".join(pattern.slot_names()) or "None")
        try:
            forms = self.project.eligible_form_names(
                pattern.id,
                pattern_override=pattern,
            )
            values = [RANDOM_VALUE] + forms if forms else [FIXED_VALUE]
            self.preview_form_combo["values"] = values
            if self.preview_form_var.get() not in values:
                self.preview_form_var.set(values[0])
            selected = self.preview_form_var.get()
            form_name = (
                None
                if selected in {RANDOM_VALUE, FIXED_VALUE}
                else selected
            )
            value = self.project.build_preview(
                pattern.id,
                form_name=form_name,
                pattern_override=pattern,
                rng=random.Random(self.preview_seed),
            )
            self.preview_text.insert(
                "1.0",
                value.live_example(),
            )
        except ProjectError as exception:
            self.preview_form_combo["values"] = [FIXED_VALUE]
            self.preview_form_var.set(FIXED_VALUE)
            self.preview_text.insert("1.0", f"Cannot preview yet: {exception}")


class CategoryHelpTab(EditorTab):
    def __init__(self, parent: tk.Widget, app: "QuizStudio") -> None:
        super().__init__(parent, app)
        text = tk.Text(
            self,
            wrap="word",
            relief="flat",
            background="#f7f8fa",
            padx=18,
            pady=16,
        )
        scrollbar = ttk.Scrollbar(self, orient="vertical", command=text.yview)
        text.configure(yscrollcommand=scrollbar.set)
        text.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")
        lines = [
            "V6 PATTERNS AND TAG PROFILES\n",
            "A sentence pattern contains only a Polish question and Japanese "
            "answer. The application detects slots and resolves shared tags "
            "automatically.\n",
            "DYNAMIC QUIZ FORM\n",
            "{verb.form}                 conjugate this verb",
            "{adjective.form}            conjugate this adjective",
            "{copula.form}               use a form from copulas.xml",
            "{reporting_verb.form}       verb tagged reporting",
            "{progressive_auxiliary.form} auxiliary tagged "
            "progressive_auxiliary\n",
            "A pattern may contain one dynamic .form target. The simulator "
            "offers the eight core non-past/past, affirmative/negative, "
            "plain/polite forms.\n",
            "FIXED GRAMMAR FORMS\n",
            "{adjective.attributive}     い form or な form before a noun",
            "{verb.dictionary}           dictionary form",
            "{verb.te}                   て-form",
            "{predicate.plain_nonpast}   plain predicate form\n",
            "ENTITIES AND POLISH FIELDS\n",
            "{object}                    entity kana",
            "この{subject}                literal この + entity kana",
            "{object.pl_accusative}      Polish accusative",
            "{destination.pl_genitive}   Polish genitive",
            "{companion.pl_instrumental} Polish instrumental\n",
            "OPTIONAL RELATIONSHIPS\n",
            "The pattern decides which roles are required. A bare pattern "
            "with only {verb.form} never asks for an object or destination.",
            "A verb relationship only says what may fill a role when that "
            "role appears in the selected pattern.",
            "Example in the word editor:",
            "object: edible",
            "destination: land_destination",
            "companion: person\n",
            "Role names are not programmed into Python. Any entity token "
            "can become a role. If a pattern contains {destination}, the "
            "application looks for a verb relationship named destination.",
            "An adjective and its single entity share descriptive tags such "
            "as color, object_dimension, taste, sound, or personality.",
            "A named word slot supplies its own filter. For example, "
            "reporting_verb selects the tag reporting and "
            "progressive_auxiliary selects the same-named tag.",
            "i_adjective and na_adjective optionally filter adjective type.\n",
            "TAG PROFILES\n",
            "Entities normally select one reusable XML profile instead of "
            "copying many tags. Profiles may include other profiles.",
            "motorcycle -> vehicle",
            "vehicle includes physical_object",
            "The effective tags are expanded from these XML definitions. "
            "Extra entity tags are needed only for exceptions.\n",
            "GENERIC MATCHING\n",
            "1. Read the slots from the pattern.",
            "2. Select a verb that has every relationship requested by it.",
            "3. Expand the entity's XML tag profiles.",
            "4. Match relationship and adjective tags with effective tags.",
            "5. Apply the chosen form and paired context.\n",
            "EXAMPLE\n",
            "Question:\n"
            "Ułóż zdanie: {context.pl_prefix}{verb.prompt}; "
            "{adjective.prompt} {object.pl_nominative}.\n",
            "Answer:\n"
            "{context.jp_prefix}{adjective.attributive}"
            "{object}を{verb.form}\n",
            "taberu relation object: edible",
            "ringo profile: fruit",
            "fruit includes food + physical_object",
            "effective tags include edible, taste, color, and object_dimension",
            "oishii tags: taste",
            "Result: きのう、おいしいりんごをたべた\n",
            "A simple pattern ignores all relationships:",
            "{context.jp_prefix}{verb.form}",
            "Result examples: きのう、たべた。 あした、いく。",
        ]
        text.insert("1.0", "\n".join(lines))
        text.configure(state="disabled")


class SentenceMapsTab(EditorTab):
    def __init__(self, parent: tk.Widget, app: "QuizStudio") -> None:
        super().__init__(parent, app)
        notebook = ttk.Notebook(self)
        notebook.pack(fill="both", expand=True)
        self.patterns = PatternsTab(notebook, app)
        self.help = CategoryHelpTab(notebook, app)
        notebook.add(self.patterns, text="Sentence patterns")
        notebook.add(self.help, text="Token and tag help")

    def refresh(self) -> None:
        self.patterns.refresh()


class ContextsTab(EditorTab):
    def __init__(self, parent: tk.Widget, app: "QuizStudio") -> None:
        super().__init__(parent, app)
        self.selected_pool_id: Optional[str] = None
        self.selected_option_id: Optional[str] = None
        pane = ttk.Panedwindow(self, orient="horizontal")
        pane.pack(fill="both", expand=True)
        left = ttk.Frame(pane, padding=(0, 0, 12, 0))
        right = ttk.Frame(pane)
        pane.add(left, weight=2)
        pane.add(right, weight=3)

        ttk.Label(
            left,
            text="Context pools",
            style="Section.TLabel",
        ).pack(anchor="w", pady=(0, 8))
        self.pool_tree = ttk.Treeview(
            left,
            columns=("id", "label", "options"),
            show="headings",
        )
        for name, title, width in (
            ("id", "ID", 110),
            ("label", "Label", 160),
            ("options", "Options", 70),
        ):
            self.pool_tree.heading(name, text=title)
            self.pool_tree.column(name, width=width, anchor="w")
        self.pool_tree.pack(fill="both", expand=True)
        self.pool_tree.bind("<<TreeviewSelect>>", self._select_pool)

        pool_box = ttk.LabelFrame(right, text="Context pool", padding=12)
        pool_box.pack(fill="x")
        pool_box.columnconfigure(1, weight=1)
        pool_box.columnconfigure(3, weight=1)
        self.pool_id_var = tk.StringVar()
        self.pool_label_var = tk.StringVar()
        entry(pool_box, 0, "Pool ID", self.pool_id_var)
        entry(pool_box, 0, "Label", self.pool_label_var, 2)
        button_row(
            pool_box,
            1,
            [
                ("New pool", self.new_pool),
                ("Save pool", self.save_pool),
                ("Delete pool", self.delete_pool),
            ],
        )

        options = ttk.LabelFrame(
            right,
            text="Random paired options",
            padding=12,
        )
        options.pack(fill="both", expand=True, pady=(12, 0))
        options.columnconfigure(0, weight=1)
        options.rowconfigure(0, weight=1)
        self.option_tree = ttk.Treeview(
            options,
            columns=("id", "pl", "jp", "weight"),
            show="headings",
            height=8,
        )
        for name, title, width in (
            ("id", "Option ID", 120),
            ("pl", "Polish prefix", 220),
            ("jp", "Japanese prefix", 180),
            ("weight", "Weight", 70),
        ):
            self.option_tree.heading(name, text=title)
            self.option_tree.column(name, width=width, anchor="w")
        self.option_tree.grid(
            row=0,
            column=0,
            columnspan=4,
            sticky="nsew",
        )
        self.option_tree.bind("<<TreeviewSelect>>", self._select_option)
        self.option_id_var = tk.StringVar()
        self.pl_var = tk.StringVar()
        self.jp_var = tk.StringVar()
        self.weight_var = tk.StringVar(value="1")
        entry(options, 1, "Option ID", self.option_id_var)
        entry(options, 1, "Weight", self.weight_var, 2)
        entry(options, 2, "Polish prefix", self.pl_var)
        entry(options, 2, "Japanese prefix", self.jp_var, 2)
        button_row(
            options,
            3,
            [
                ("New option", self.new_option),
                ("Save option", self.save_option),
                ("Delete option", self.delete_option),
            ],
        )

    def refresh(self) -> None:
        self.pool_tree.delete(*self.pool_tree.get_children())
        for pool in self.project.contexts.values():
            self.pool_tree.insert(
                "",
                "end",
                iid=pool.id,
                values=(pool.id, pool.label, len(pool.options)),
            )
        if self.selected_pool_id and self.pool_tree.exists(
            self.selected_pool_id
        ):
            self.pool_tree.selection_set(self.selected_pool_id)
        elif self.project.contexts:
            self.pool_tree.selection_set(next(iter(self.project.contexts)))
            self._select_pool()

    def _select_pool(self, _event: object = None) -> None:
        selection = self.pool_tree.selection()
        if not selection:
            return
        pool = self.project.contexts[selection[0]]
        self.selected_pool_id = pool.id
        self.pool_id_var.set(pool.id)
        self.pool_label_var.set(pool.label)
        self.selected_option_id = None
        self._refresh_options()
        self.new_option()

    def _refresh_options(self) -> None:
        self.option_tree.delete(*self.option_tree.get_children())
        pool = self.project.contexts.get(self.selected_pool_id or "")
        if not pool:
            return
        for option in pool.options.values():
            self.option_tree.insert(
                "",
                "end",
                iid=option.id,
                values=(
                    option.id,
                    option.pl_prefix,
                    option.jp_prefix,
                    option.weight,
                ),
            )

    def new_pool(self) -> None:
        self.selected_pool_id = None
        self.pool_id_var.set("")
        self.pool_label_var.set("")
        self.selected_option_id = None
        self._refresh_options()

    def save_pool(self) -> None:
        pool_id = self.pool_id_var.get().strip()
        if not pool_id:
            messagebox.showerror(
                APP_NAME,
                "Context pool ID is required.",
                parent=self,
            )
            return
        old_pool = self.project.contexts.get(self.selected_pool_id or "")
        options = old_pool.options if old_pool else {}
        if self.selected_pool_id and self.selected_pool_id != pool_id:
            old_id = self.selected_pool_id
            self.project.contexts.pop(old_id, None)
            for dictionary in self.project.words.values():
                for word in dictionary.values():
                    for form in word.forms.values():
                        if form.context == old_id:
                            form.context = pool_id
        self.project.contexts[pool_id] = ContextPool(
            id=pool_id,
            label=self.pool_label_var.get().strip() or pool_id,
            options=options,
        )
        self.selected_pool_id = pool_id
        self.changed(f"Saved context pool {pool_id}.")
        self.app.refresh_all()

    def delete_pool(self) -> None:
        if self.selected_pool_id == "none":
            messagebox.showerror(
                APP_NAME,
                "The required 'none' pool cannot be deleted.",
                parent=self,
            )
            return
        references = []
        for dictionary in self.project.words.values():
            for word in dictionary.values():
                for form in word.forms.values():
                    if form.context == self.selected_pool_id:
                        references.append(f"{word.id}/{form.name}")
        if references:
            messagebox.showerror(
                APP_NAME,
                "This pool is still used by forms:\n"
                + "\n".join(references[:12]),
                parent=self,
            )
            return
        if self.selected_pool_id and messagebox.askyesno(
            APP_NAME,
            f"Delete context pool '{self.selected_pool_id}'?",
            parent=self,
        ):
            pool_id = self.selected_pool_id
            self.project.contexts.pop(pool_id, None)
            self.new_pool()
            self.changed(f"Deleted context pool {pool_id}.")
            self.app.refresh_all()

    def _select_option(self, _event: object = None) -> None:
        selection = self.option_tree.selection()
        pool = self.project.contexts.get(self.selected_pool_id or "")
        if not selection or not pool:
            return
        option = pool.options[selection[0]]
        self.selected_option_id = option.id
        self.option_id_var.set(option.id)
        self.pl_var.set(option.pl_prefix)
        self.jp_var.set(option.jp_prefix)
        self.weight_var.set(str(option.weight))

    def new_option(self) -> None:
        self.selected_option_id = None
        self.option_id_var.set("")
        self.pl_var.set("")
        self.jp_var.set("")
        self.weight_var.set("1")

    def save_option(self) -> None:
        pool = self.project.contexts.get(self.selected_pool_id or "")
        if not pool:
            messagebox.showerror(
                APP_NAME,
                "Select or save a context pool first.",
                parent=self,
            )
            return
        option_id = self.option_id_var.get().strip()
        try:
            weight = int(self.weight_var.get())
            if weight < 1:
                raise ValueError
        except ValueError:
            messagebox.showerror(
                APP_NAME,
                "Weight must be a positive integer.",
                parent=self,
            )
            return
        if not option_id:
            messagebox.showerror(
                APP_NAME,
                "Option ID is required.",
                parent=self,
            )
            return
        if self.selected_option_id and self.selected_option_id != option_id:
            pool.options.pop(self.selected_option_id, None)
        pool.options[option_id] = ContextOption(
            id=option_id,
            pl_prefix=self.pl_var.get(),
            jp_prefix=self.jp_var.get(),
            weight=weight,
        )
        self.selected_option_id = option_id
        self.changed(f"Saved context option {pool.id}/{option_id}.")
        self.app.refresh_all()

    def delete_option(self) -> None:
        pool = self.project.contexts.get(self.selected_pool_id or "")
        if pool and self.selected_option_id:
            option_id = self.selected_option_id
            pool.options.pop(option_id, None)
            self.new_option()
            self.changed(f"Deleted context option {pool.id}/{option_id}.")
            self.app.refresh_all()


class SimulatorTab(EditorTab):
    def __init__(self, parent: tk.Widget, app: "QuizStudio") -> None:
        super().__init__(parent, app)
        self.current_preview = None
        self.locked_choices: Dict[str, str] = {}
        self.slot_order: List[str] = []
        self.slot_vars: Dict[str, tk.StringVar] = {}
        self.slot_combos: Dict[str, ttk.Combobox] = {}
        self.slot_counts: Dict[str, ttk.Label] = {}
        self.slot_display_to_id: Dict[str, Dict[str, str]] = {}
        self.slot_id_to_display: Dict[str, Dict[str, str]] = {}

        controls = ttk.Frame(self)
        controls.pack(fill="x", pady=(0, 12))
        controls.columnconfigure(2, weight=1)
        self.pattern_var = tk.StringVar()
        self.form_var = tk.StringVar(value=RANDOM_VALUE)

        ttk.Label(controls, text="Sentence pattern").grid(
            row=0,
            column=0,
            sticky="w",
        )
        self.pattern_combo = ttk.Combobox(
            controls,
            textvariable=self.pattern_var,
            state="readonly",
            width=34,
        )
        self.pattern_combo.grid(
            row=1,
            column=0,
            sticky="ew",
            padx=(0, 18),
            pady=(3, 4),
        )
        ttk.Label(controls, text="Quiz form").grid(
            row=2,
            column=0,
            sticky="w",
        )
        self.form_combo = ttk.Combobox(
            controls,
            textvariable=self.form_var,
            state="readonly",
            width=34,
        )
        self.form_combo.grid(
            row=3,
            column=0,
            sticky="ew",
            padx=(0, 18),
            pady=(3, 0),
        )
        preview_actions = ttk.Frame(controls)
        preview_actions.grid(
            row=1,
            column=1,
            sticky="ew",
            padx=(0, 18),
            pady=(3, 4),
        )
        ttk.Button(
            preview_actions,
            text="Generate preview",
            style="Accent.TButton",
            command=self.generate,
            width=20,
        ).pack(side="left")
        ttk.Button(
            preview_actions,
            text="Count possibilities",
            command=self.count_possibilities,
            width=20,
        ).pack(side="left", padx=(8, 0))

        secondary_actions = ttk.Frame(controls)
        secondary_actions.grid(
            row=3,
            column=1,
            sticky="ew",
            padx=(0, 18),
            pady=(3, 0),
        )
        ttk.Button(
            secondary_actions,
            text="Random preview",
            command=self.random_preview,
            width=20,
        ).pack(side="left")
        ttk.Button(
            secondary_actions,
            text="Refresh",
            command=self.reset_options,
            width=20,
        ).pack(side="left", padx=(8, 0))

        statistics = ttk.LabelFrame(
            controls,
            text="Possibility counter",
            padding=8,
        )
        statistics.grid(
            row=0,
            column=2,
            rowspan=4,
            sticky="nsew",
        )
        statistics_body = ttk.Frame(statistics)
        statistics_body.pack(fill="both", expand=True)
        self.statistics_text = tk.Text(
            statistics_body,
            height=3,
            wrap="word",
            relief="flat",
            background="#f7f8fa",
        )
        statistics_scrollbar = ttk.Scrollbar(
            statistics_body,
            orient="vertical",
            command=self.statistics_text.yview,
        )
        self.statistics_text.configure(
            yscrollcommand=statistics_scrollbar.set
        )
        self.statistics_text.pack(
            side="left",
            fill="both",
            expand=True,
        )
        statistics_scrollbar.pack(side="right", fill="y")
        self.statistics_text.insert(
            "1.0",
            "Click Count possibilities to analyze the current selections.",
        )
        self.statistics_text.configure(state="disabled")

        self.pattern_combo.bind(
            "<<ComboboxSelected>>",
            self._pattern_changed,
        )
        self.form_combo.bind(
            "<<ComboboxSelected>>",
            self._form_changed,
        )

        choices = ttk.LabelFrame(
            self,
            text="Pattern options",
            padding=8,
        )
        choices.pack(fill="x", pady=(0, 12))
        choices.columnconfigure(1, weight=1)
        self.options_frame = choices
        ttk.Label(
            choices,
            text=(
                "Manual choices stay selected until Refresh. Compatible fields "
                "are filled automatically, and the displayed values are used "
                "for the preview."
            ),
            style="Muted.TLabel",
        ).grid(
            row=99,
            column=0,
            columnspan=3,
            sticky="w",
            pady=(8, 0),
        )
        self.options_hint = choices.grid_slaves(row=99)[0]

        preview_box = ttk.LabelFrame(
            self,
            text="Preview",
            padding=8,
        )
        preview_box.pack(fill="x", pady=(0, 12))
        self.preview_text = tk.Text(
            preview_box,
            height=6,
            wrap="word",
            relief="flat",
            background="#f7f8fa",
            font=("Segoe UI", 11),
        )
        self.preview_text.pack(fill="x")
        self.preview_text.insert(
            "1.0",
            "Choose a pattern and form, then generate one preview.",
        )
        self.preview_text.configure(state="disabled")

        trace_box = ttk.LabelFrame(
            self,
            text="Build trace",
            padding=8,
        )
        trace_box.pack(fill="both", expand=True)
        self.trace = tk.Text(
            trace_box,
            height=10,
            wrap="none",
            relief="flat",
            background="#f7f8fa",
        )
        self.trace.pack(fill="both", expand=True)

    def refresh(self) -> None:
        pattern_ids = list(self.project.patterns)
        self.pattern_combo["values"] = pattern_ids
        if pattern_ids and self.pattern_var.get() not in pattern_ids:
            self.pattern_var.set(pattern_ids[0])
        self._pattern_changed()

    def _pattern_changed(self, _event: object = None) -> None:
        pattern_id = self.pattern_var.get()
        if not pattern_id or pattern_id not in self.project.patterns:
            self.form_combo["values"] = [FIXED_VALUE]
            self.form_var.set(FIXED_VALUE)
            self._rebuild_slot_menus()
            return
        try:
            forms = self.project.eligible_form_names(pattern_id)
        except ProjectError:
            forms = []
        values = [RANDOM_VALUE] + forms if forms else [FIXED_VALUE]
        self.form_combo["values"] = values
        if self.form_var.get() not in values:
            self.form_var.set(values[0])
        self._rebuild_slot_menus()
        self.random_preview()

    def _form_changed(self, _event: object = None) -> None:
        self._clear_statistics()
        self.generate()

    def _rebuild_slot_menus(self) -> None:
        for widget in list(self.options_frame.grid_slaves()):
            if widget is not self.options_hint:
                widget.destroy()
        self.slot_order = []
        self.slot_vars.clear()
        self.slot_combos.clear()
        self.slot_counts.clear()
        self.slot_display_to_id.clear()
        self.slot_id_to_display.clear()
        self.locked_choices.clear()

        pattern_id = self.pattern_var.get()
        if not pattern_id or pattern_id not in self.project.patterns:
            return
        try:
            analysis = self.project.analyze_pattern(pattern_id)
            known_slots = set(analysis.word_slots) | set(
                analysis.entity_slots
            )
            self.slot_order = [
                slot_id
                for slot_id in self.project.patterns[
                    pattern_id
                ].slot_names()
                if slot_id in known_slots
            ]
            for row, slot_id in enumerate(self.slot_order):
                ttk.Label(
                    self.options_frame,
                    text=slot_id.replace("_", " ").title(),
                ).grid(
                    row=row,
                    column=0,
                    sticky="w",
                    padx=(0, 8),
                    pady=3,
                )
                variable = tk.StringVar(value="")
                combo = ttk.Combobox(
                    self.options_frame,
                    textvariable=variable,
                    state="readonly",
                    width=62,
                )
                combo.grid(
                    row=row,
                    column=1,
                    sticky="ew",
                    pady=3,
                )
                combo.bind(
                    "<<ComboboxSelected>>",
                    lambda _event, selected_slot=slot_id:
                        self._slot_changed(selected_slot),
                )
                count = ttk.Label(
                    self.options_frame,
                    text="",
                    style="Muted.TLabel",
                )
                count.grid(
                    row=row,
                    column=2,
                    sticky="w",
                    padx=(8, 0),
                )
                self.slot_vars[slot_id] = variable
                self.slot_combos[slot_id] = combo
                self.slot_counts[slot_id] = count
                self.slot_display_to_id[slot_id] = {}
                self.slot_id_to_display[slot_id] = {}
            self._refresh_slot_options()
            self._clear_preview()
            self._clear_statistics()
        except ProjectError as exception:
            self._show_error(exception)

    def reset_options(self) -> None:
        self.locked_choices.clear()
        for variable in self.slot_vars.values():
            variable.set("")
        self._refresh_slot_options()
        self._clear_preview()
        self._clear_statistics()
        self.app.set_status("Simulator choices reset.")

    def random_preview(self) -> None:
        self.locked_choices.clear()
        for variable in self.slot_vars.values():
            variable.set("")
        self._clear_statistics()
        self.generate()

    def _slot_changed(self, slot_id: str) -> None:
        selected_id = self.slot_display_to_id.get(
            slot_id,
            {},
        ).get(self.slot_vars[slot_id].get(), "")
        if not selected_id:
            previous_choices = dict(self.locked_choices)
            self.locked_choices.pop(slot_id, None)
            try:
                self._complete_from_locked_choices()
            except ProjectError as exception:
                self.locked_choices = previous_choices
                self._complete_from_locked_choices()
                self._show_error(exception)
            return
        previous_choices = dict(self.locked_choices)
        self.locked_choices[slot_id] = selected_id
        try:
            self._complete_from_locked_choices()
            self.app.set_status(
                f"Selected {slot_id}={selected_id}; previous choices kept."
            )
        except ProjectError as exception:
            self.locked_choices = previous_choices
            self._complete_from_locked_choices()
            self._show_error(exception)

    def _complete_from_locked_choices(self) -> None:
        analysis = self.project.analyze_pattern(
            self.pattern_var.get()
        )
        word_choices, entity_choices = self._locked_slot_choices(
            analysis
        )
        completed = self.project.complete_choices(
            self.pattern_var.get(),
            form_name=self._selected_form(),
            word_choices=word_choices,
            entity_choices=entity_choices,
            rng=random.Random(),
        )
        self._apply_completed_choices(
            completed.words,
            completed.entities,
        )
        self._refresh_slot_options()
        self._clear_preview()
        self._clear_statistics()

    def _refresh_slot_options(self) -> None:
        pattern_id = self.pattern_var.get()
        if not self.slot_order or not pattern_id:
            return
        try:
            analysis = self.project.analyze_pattern(pattern_id)
            selected_words, selected_entities = self._selected_choices(
                analysis
            )
            for slot_id in self.slot_order:
                word_choices: Dict[str, str] = {}
                entity_choices: Dict[str, str] = {}
                for locked_slot, item_id in self.locked_choices.items():
                    if locked_slot == slot_id:
                        continue
                    if locked_slot in analysis.word_slots:
                        word_choices[locked_slot] = item_id
                    elif locked_slot in analysis.entity_slots:
                        entity_choices[locked_slot] = item_id
                choices = self.project.compatible_choices(
                    pattern_id,
                    form_name=self._selected_form(),
                    word_choices=word_choices,
                    entity_choices=entity_choices,
                )
                items = (
                    choices.words[slot_id]
                    if slot_id in analysis.word_slots
                    else choices.entities[slot_id]
                )
                current_id = (
                    selected_words.get(slot_id)
                    or selected_entities.get(slot_id)
                    or ""
                )
                display_to_id: Dict[str, str] = {}
                id_to_display: Dict[str, str] = {}
                for item in items:
                    display = self._choice_label(
                        item,
                        slot_id in analysis.word_slots,
                    )
                    display_to_id[display] = item.id
                    id_to_display[item.id] = display
                self.slot_display_to_id[slot_id] = display_to_id
                self.slot_id_to_display[slot_id] = id_to_display
                self.slot_combos[slot_id]["values"] = [
                    "",
                    *display_to_id,
                ]
                self.slot_vars[slot_id].set(
                    id_to_display.get(current_id, "")
                )
                self.slot_counts[slot_id].configure(
                    text=(
                        f"{len(items):,} choices"
                        + (
                            " • selected"
                            if slot_id in self.locked_choices
                            else ""
                        )
                    )
                )
        except ProjectError as exception:
            self._show_error(exception)

    @staticmethod
    def _choice_label(item: object, is_word: bool) -> str:
        kana = getattr(item, "kana", "")
        item_id = getattr(item, "id", "")
        if is_word:
            meaning = getattr(item, "translation", "")
        else:
            meaning = getattr(item, "polish", {}).get(
                "nominative",
                "",
            )
        parts = [value for value in (kana, meaning) if value]
        description = " | ".join(parts) or item_id
        return f"{description} [{item_id}]"

    def _selected_choices(
        self,
        analysis: object,
    ) -> tuple[Dict[str, str], Dict[str, str]]:
        words: Dict[str, str] = {}
        entities: Dict[str, str] = {}
        for slot_id in self.slot_order:
            item_id = self.slot_display_to_id.get(
                slot_id,
                {},
            ).get(self.slot_vars[slot_id].get(), "")
            if not item_id:
                continue
            if slot_id in analysis.word_slots:
                words[slot_id] = item_id
            elif slot_id in analysis.entity_slots:
                entities[slot_id] = item_id
        return words, entities

    def _locked_slot_choices(
        self,
        analysis: object,
    ) -> tuple[Dict[str, str], Dict[str, str]]:
        words = {
            slot_id: item_id
            for slot_id, item_id in self.locked_choices.items()
            if slot_id in analysis.word_slots
        }
        entities = {
            slot_id: item_id
            for slot_id, item_id in self.locked_choices.items()
            if slot_id in analysis.entity_slots
        }
        return words, entities

    def _apply_completed_choices(
        self,
        words: Dict[str, str],
        entities: Dict[str, str],
    ) -> None:
        selected = dict(words)
        selected.update(entities)
        analysis = self.project.analyze_pattern(self.pattern_var.get())
        for slot_id, variable in self.slot_vars.items():
            item_id = selected.get(slot_id, "")
            display = self.slot_id_to_display.get(
                slot_id,
                {},
            ).get(item_id, "")
            if item_id and not display:
                if slot_id in analysis.word_slots:
                    dictionary_id = self.project._dictionary_for_slot(
                        slot_id
                    )
                    item = self.project.words[dictionary_id][item_id]
                    is_word = True
                else:
                    item = self.project.entities[item_id]
                    is_word = False
                display = self._choice_label(item, is_word)
                self.slot_display_to_id[slot_id][display] = item_id
                self.slot_id_to_display[slot_id][item_id] = display
            variable.set(display)

    def generate(self) -> None:
        pattern_id = self.pattern_var.get()
        if not pattern_id:
            return
        try:
            analysis = self.project.analyze_pattern(pattern_id)
            word_choices, entity_choices = self._selected_choices(
                analysis
            )
            rng = random.Random()
            completed = self.project.complete_choices(
                pattern_id,
                form_name=self._selected_form(),
                word_choices=word_choices,
                entity_choices=entity_choices,
                rng=rng,
            )
            self.current_preview = self.project.build_preview(
                pattern_id,
                form_name=completed.form_name or None,
                word_choices=completed.words,
                entity_choices=completed.entities,
                rng=rng,
            )
            self._apply_completed_choices(
                completed.words,
                completed.entities,
            )
            self._refresh_slot_options()
            self._show_preview()
            self.app.set_status("Generated one sentence preview.")
        except ProjectError as exception:
            self._show_error(exception)

    def count_possibilities(self) -> None:
        pattern_id = self.pattern_var.get()
        if not pattern_id:
            return
        try:
            analysis = self.project.analyze_pattern(pattern_id)
            word_choices, entity_choices = self._locked_slot_choices(
                analysis
            )
            stats = self.project.count_possibilities(
                pattern_id,
                form_name=self._selected_form(),
                word_choices=word_choices,
                entity_choices=entity_choices,
            )
            self.statistics_text.configure(state="normal")
            self.statistics_text.delete("1.0", "end")
            self.statistics_text.insert("1.0", stats.summary())
            self.statistics_text.configure(state="disabled")
            self.app.set_status(
                f"Counted {stats.generated_sentences:,} possibilities."
            )
        except ProjectError as exception:
            self._show_error(exception)

    def _selected_form(self) -> Optional[str]:
        selected = self.form_var.get()
        return (
            None
            if selected in {RANDOM_VALUE, FIXED_VALUE}
            else selected
        )

    def _show_preview(self) -> None:
        if self.current_preview is None:
            return
        preview = self.current_preview
        self.preview_text.configure(state="normal")
        self.preview_text.delete("1.0", "end")
        self.preview_text.insert(
            "1.0",
            "Question\n"
            + preview.source
            + "\n\nExpected Japanese\n"
            + preview.answer,
        )
        self.preview_text.configure(state="disabled")
        self.trace.delete("1.0", "end")
        self.trace.insert("1.0", preview.trace())

    def _clear_preview(self) -> None:
        self.current_preview = None
        self.preview_text.configure(state="normal")
        self.preview_text.delete("1.0", "end")
        self.preview_text.insert(
            "1.0",
            "Press Generate preview to build one sentence.",
        )
        self.preview_text.configure(state="disabled")
        self.trace.delete("1.0", "end")

    def _clear_statistics(self) -> None:
        self.statistics_text.configure(state="normal")
        self.statistics_text.delete("1.0", "end")
        self.statistics_text.insert(
            "1.0",
            "Click Count possibilities to analyze the current selections.",
        )
        self.statistics_text.configure(state="disabled")

    def _show_error(self, exception: Exception) -> None:
        messagebox.showerror(APP_NAME, str(exception), parent=self)


class QuizStudio(tk.Tk):
    def __init__(self, manifest_path: Path) -> None:
        super().__init__()
        self.title(APP_NAME)
        self.geometry("1280x820")
        self.minsize(1050, 700)
        self.configure(background="#f4f5f7")
        self.dirty = False
        self.project = QuizProject.load(manifest_path)
        self._configure_style()
        self.protocol("WM_DELETE_WINDOW", self.close)
        self.bind("<Control-s>", lambda _event: self.save_all())
        self.bind("<Control-o>", lambda _event: self.open_project())

        header = ttk.Frame(self, style="Header.TFrame", padding=(18, 14))
        header.pack(fill="x")
        ttk.Label(
            header,
            text=APP_NAME,
            style="Title.TLabel",
        ).pack(side="left")
        self.project_label = ttk.Label(
            header,
            text=str(manifest_path),
            style="HeaderMuted.TLabel",
        )
        self.project_label.pack(side="left", padx=(18, 0))
        ttk.Button(
            header,
            text="Open",
            command=self.open_project,
        ).pack(side="right", padx=(8, 0))
        ttk.Button(
            header,
            text="Validate",
            command=self.validate_project,
        ).pack(side="right", padx=(8, 0))
        ttk.Button(
            header,
            text="Save all",
            style="Accent.TButton",
            command=self.save_all,
        ).pack(side="right", padx=(8, 0))

        self.notebook = ttk.Notebook(self)
        self.notebook.pack(fill="both", expand=True, padx=12, pady=12)
        self.words_tab = WordsTab(self.notebook, self)
        self.entities_tab = SentenceVocabularyTab(self.notebook, self)
        self.maps_tab = SentenceMapsTab(self.notebook, self)
        self.contexts_tab = ContextsTab(self.notebook, self)
        self.simulator_tab = SimulatorTab(self.notebook, self)
        self.tabs: List[EditorTab] = [
            self.words_tab,
            self.entities_tab,
            self.maps_tab,
            self.contexts_tab,
            self.simulator_tab,
        ]
        self.notebook.add(self.words_tab, text="Words")
        self.notebook.add(
            self.entities_tab,
            text="Sentence vocabulary",
        )
        self.notebook.add(self.maps_tab, text="Sentence maps")
        self.notebook.add(self.contexts_tab, text="Contexts")
        self.notebook.add(self.simulator_tab, text="Simulator")

        footer = ttk.Frame(self, padding=(14, 6))
        footer.pack(fill="x")
        self.status_var = tk.StringVar(value="Project loaded.")
        ttk.Label(
            footer,
            textvariable=self.status_var,
            style="Muted.TLabel",
        ).pack(side="left")
        ttk.Label(
            footer,
            text="Ctrl+S save    Ctrl+O open",
            style="Muted.TLabel",
        ).pack(side="right")
        self.refresh_all()

    def _configure_style(self) -> None:
        style = ttk.Style(self)
        style.theme_use("clam")
        background = "#f4f5f7"
        panel = "#ffffff"
        ink = "#172033"
        muted = "#667085"
        accent = "#d97706"
        style.configure(
            ".",
            font=("Segoe UI", 10),
            background=background,
            foreground=ink,
        )
        style.configure("TFrame", background=background)
        style.configure("Header.TFrame", background="#172033")
        style.configure("TLabel", background=background, foreground=ink)
        style.configure(
            "Title.TLabel",
            background="#172033",
            foreground="white",
            font=("Segoe UI Semibold", 16),
        )
        style.configure(
            "HeaderMuted.TLabel",
            background="#172033",
            foreground="#cbd5e1",
        )
        style.configure(
            "Section.TLabel",
            font=("Segoe UI Semibold", 12),
            foreground=ink,
        )
        style.configure("Muted.TLabel", foreground=muted)
        style.configure(
            "TLabelframe",
            background=panel,
            bordercolor="#d8dde6",
            relief="solid",
        )
        style.configure(
            "TLabelframe.Label",
            background=panel,
            foreground=ink,
            font=("Segoe UI Semibold", 10),
        )
        style.configure("TButton", padding=(12, 7))
        style.configure(
            "Accent.TButton",
            background=accent,
            foreground="white",
            bordercolor=accent,
        )
        style.map("Accent.TButton", background=[("active", "#b45309")])
        style.configure("TNotebook", background=background, borderwidth=0)
        style.configure(
            "TNotebook.Tab",
            padding=(16, 9),
            background="#e7eaf0",
        )
        style.map(
            "TNotebook.Tab",
            background=[("selected", panel)],
            foreground=[("selected", ink)],
        )
        style.configure(
            "Treeview",
            background=panel,
            fieldbackground=panel,
            rowheight=28,
            bordercolor="#d8dde6",
        )
        style.configure(
            "Treeview.Heading",
            background="#eef1f5",
            foreground=ink,
            font=("Segoe UI Semibold", 9),
        )

    def refresh_all(self) -> None:
        for tab in self.tabs:
            tab.refresh()

    def mark_changed(self, message: str) -> None:
        self.dirty = True
        self.set_status(message + " Changes are not written to XML yet.")

    def set_status(self, message: str) -> None:
        self.status_var.set(message)

    def save_all(self) -> None:
        issues = self.project.validate()
        errors = [issue for issue in issues if issue.startswith("ERROR:")]
        if errors and not messagebox.askyesno(
            APP_NAME,
            "The project has validation errors. Save it anyway?\n\n"
            + "\n".join(errors[:8]),
            parent=self,
        ):
            return
        try:
            self.project.save()
            self.dirty = False
            self.set_status("All XML files saved.")
        except (OSError, ProjectError) as exception:
            messagebox.showerror(APP_NAME, str(exception), parent=self)

    def validate_project(self) -> None:
        issues = self.project.validate()
        if issues:
            messagebox.showwarning(
                APP_NAME,
                "\n".join(issues[:30]),
                parent=self,
            )
        else:
            messagebox.showinfo(
                APP_NAME,
                "Validation passed. No issues found.",
                parent=self,
            )
        self.set_status(
            f"Validation completed with {len(issues)} issue(s)."
        )

    def open_project(self) -> None:
        if self.dirty and not messagebox.askyesno(
            APP_NAME,
            "Discard unsaved changes and open another project?",
            parent=self,
        ):
            return
        selected = filedialog.askopenfilename(
            parent=self,
            title="Open quiz project",
            filetypes=[("XML project", "*.xml"), ("All files", "*.*")],
        )
        if not selected:
            return
        try:
            self.project = QuizProject.load(selected)
            self.project_label.configure(text=selected)
            self.dirty = False
            self.refresh_all()
            self.set_status("Project loaded.")
        except ProjectError as exception:
            messagebox.showerror(APP_NAME, str(exception), parent=self)

    def close(self) -> None:
        if self.dirty and not messagebox.askyesno(
            APP_NAME,
            "Close without saving the XML changes?",
            parent=self,
        ):
            return
        self.destroy()


def default_manifest() -> Path:
    return Path(__file__).resolve().parent / "../QuizFiles" / "quiz_project.xml"


def main() -> None:
    parser = argparse.ArgumentParser(description=APP_NAME)
    parser.add_argument(
        "project",
        nargs="?",
        type=Path,
        default=default_manifest(),
    )
    parser.add_argument(
        "--validate",
        action="store_true",
        help="validate the project without opening the GUI",
    )
    arguments = parser.parse_args()
    try:
        project = QuizProject.load(arguments.project)
        if arguments.validate:
            issues = project.validate()
            for issue in issues:
                print(issue)
            if any(issue.startswith("ERROR:") for issue in issues):
                raise SystemExit(1)
            print("Validation passed.")
            return
    except ProjectError as exception:
        if arguments.validate:
            print(f"ERROR: {exception}")
            raise SystemExit(1) from exception
        raise
    app = QuizStudio(arguments.project)
    app.mainloop()


if __name__ == "__main__":
    main()
