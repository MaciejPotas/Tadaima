import unittest

from app import (
    format_relations,
    parse_choices,
    parse_relations,
    sort_text_items,
)
from quiz_core import ProjectError


class SortTextItemsTests(unittest.TestCase):
    def test_sort_is_case_insensitive(self) -> None:
        records = [
            {"type": "Na_adjective"},
            {"type": "godan"},
            {"type": "ICHIDAN"},
        ]
        values = sort_text_items(records, lambda record: record["type"])
        self.assertEqual(
            ["godan", "ICHIDAN", "Na_adjective"],
            [record["type"] for record in values],
        )

    def test_sort_can_be_reversed(self) -> None:
        self.assertEqual(
            ["su", "mu", "bu"],
            sort_text_items(
                ["bu", "su", "mu"],
                lambda ending: ending,
                reverse=True,
            ),
        )

    def test_equal_values_keep_existing_order(self) -> None:
        records = [
            ("first", "godan"),
            ("second", "GODAN"),
            ("third", "ichidan"),
        ]
        values = sort_text_items(records, lambda record: record[1])
        self.assertEqual(
            ["first", "second", "third"],
            [record[0] for record in values],
        )


class ChoiceParserTests(unittest.TestCase):
    def test_fixed_choices_use_detected_slot_names(self) -> None:
        self.assertEqual(
            {
                "verb": "taberu",
                "object": "ringo",
                "adjective": "oishii",
            },
            parse_choices(
                "verb: taberu\nobject: ringo\nadjective: oishii"
            ),
        )

    def test_invalid_choice_is_rejected(self) -> None:
        with self.assertRaises(ProjectError):
            parse_choices("taberu")


class RelationParserTests(unittest.TestCase):
    def test_relations_use_role_and_shared_tags(self) -> None:
        relations = parse_relations(
            "object: food, meal\ndestination: place"
        )
        self.assertEqual(
            {
                "object": ["food", "meal"],
                "destination": ["place"],
            },
            relations,
        )
        self.assertEqual(
            "object: food, meal\ndestination: place",
            format_relations(relations),
        )

    def test_relation_without_tags_is_rejected(self) -> None:
        with self.assertRaises(ProjectError):
            parse_relations("object:")


if __name__ == "__main__":
    unittest.main()
