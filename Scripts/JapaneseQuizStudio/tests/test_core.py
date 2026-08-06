from pathlib import Path
import random
import tempfile
import unittest
import xml.etree.ElementTree as ET

from quiz_core import ProjectError, QuizProject, SentencePattern


PROJECT = Path(__file__).resolve().parents[1] / "examples" / "quiz_project.xml"


class QuizProjectTests(unittest.TestCase):
    def setUp(self) -> None:
        self.project = QuizProject.load(PROJECT)

    def test_example_project_validates(self) -> None:
        self.assertEqual([], self.project.validate())

    def test_patterns_xml_contains_no_rules_or_variants(self) -> None:
        root = ET.parse(self.project.maps_path).getroot()
        self.assertIsNone(root.find("sentence_rules"))
        self.assertEqual([], root.findall(".//variant"))
        self.assertEqual(11, len(root.findall(".//sentence_pattern")))

    def test_all_dictionary_ids_are_consistent_romaji(self) -> None:
        for dictionary in self.project.words.values():
            for word_id in dictionary:
                self.assertRegex(word_id, r"^[a-z0-9_]+$")
        self.assertIn("oishii", self.project.words["adjectives"])
        self.assertIn("aoi", self.project.words["adjectives"])
        self.assertNotIn("おいしい", self.project.words["adjectives"])

    def test_dynamic_form_exposes_only_eight_core_quiz_forms(self) -> None:
        self.assertEqual(
            [
                "dictionary",
                "polite_nonpast",
                "plain_negative",
                "polite_negative",
                "past_plain",
                "past_polite",
                "past_negative_plain",
                "past_negative_polite",
            ],
            self.project.eligible_form_names("described_object_action"),
        )

    def test_taberu_all_eight_forms_need_no_sentence_rule(self) -> None:
        expected = {
            "dictionary": "おいしいりんごをたべる",
            "polite_nonpast": "おいしいりんごをたべます",
            "plain_negative": "おいしいりんごをたべない",
            "polite_negative": "おいしいりんごをたべません",
            "past_plain": "おいしいりんごをたべた",
            "past_polite": "おいしいりんごをたべました",
            "past_negative_plain": "おいしいりんごをたべなかった",
            "past_negative_polite": "おいしいりんごをたべませんでした",
        }
        for form_name, suffix in expected.items():
            with self.subTest(form=form_name):
                preview = self.project.build_preview(
                    "described_object_action",
                    form_name=form_name,
                    word_choices={"verb": "taberu", "adjective": "oishii"},
                    entity_choices={"object": "ringo"},
                    rng=random.Random(1),
                )
                self.assertTrue(preview.answer.endswith(suffix))
                self.assertEqual(form_name, preview.dynamic_form)

    def test_taberu_resolves_edible_through_xml_relation(self) -> None:
        self.assertEqual(
            ["edible"],
            self.project.words["verbs"]["taberu"].relations["object"],
        )
        preview = self.project.build_preview(
            "transitive_object",
            form_name="dictionary",
            word_choices={"verb": "taberu"},
            entity_choices={"object": "sushi"},
            rng=random.Random(1),
        )
        self.assertEqual("すしをたべる", preview.answer)

    def test_incompatible_object_is_rejected(self) -> None:
        with self.assertRaisesRegex(ProjectError, "not eligible"):
            self.project.build_preview(
                "transitive_object",
                form_name="dictionary",
                word_choices={"verb": "taberu"},
                entity_choices={"object": "car"},
            )

    def test_read_buy_and_hold_share_one_physical_text_entity(self) -> None:
        cases = {
            "yomu": "ほんをよむ",
            "kau": "ほんをかう",
            "motsu": "ほんをもつ",
        }
        for verb_id, expected in cases.items():
            with self.subTest(verb=verb_id):
                preview = self.project.build_preview(
                    "transitive_object",
                    form_name="dictionary",
                    word_choices={"verb": verb_id},
                    entity_choices={"object": "book"},
                    rng=random.Random(1),
                )
                self.assertEqual(expected, preview.answer)

    def test_i_adjective_attributive_form_is_selected_by_grammar(self) -> None:
        preview = self.project.build_preview(
            "described_object_action",
            form_name="past_plain",
            word_choices={"verb": "kau", "adjective": "aoi"},
            entity_choices={"object": "car"},
            rng=random.Random(1),
        )
        self.assertTrue(preview.answer.endswith("あおいくるまをかった"))
        self.assertEqual(
            "attributive_nonpast",
            preview.forms["adjective"],
        )

    def test_na_adjective_attributive_form_adds_na(self) -> None:
        preview = self.project.build_preview(
            "described_object_action",
            form_name="dictionary",
            word_choices={"verb": "kiku", "adjective": "shizuka"},
            entity_choices={"object": "music"},
            rng=random.Random(1),
        )
        self.assertEqual("しずかなおんがくをきく", preview.answer)

    def test_adjective_can_be_the_dynamic_quiz_target(self) -> None:
        preview = self.project.build_preview(
            "adjective_predicate",
            form_name="predicate_plain_past",
            word_choices={"adjective": "ookii"},
            entity_choices={"subject": "bag"},
            rng=random.Random(1),
        )
        self.assertEqual("ta torba była duża.", preview.source)
        self.assertEqual("このかばんはおおきかった", preview.answer)

    def test_attributive_pattern_uses_external_copula_forms(self) -> None:
        self.assertEqual(
            [
                "dictionary",
                "polite_nonpast",
                "plain_negative",
                "polite_negative",
                "past_plain",
                "past_polite",
                "past_negative_plain",
                "past_negative_polite",
            ],
            self.project.eligible_form_names(
                "adjective_attributive"
            ),
        )
        preview = self.project.build_preview(
            "adjective_attributive",
            form_name="past_negative_plain",
            word_choices={"adjective": "kirei", "copula": "da"},
            entity_choices={"subject": "home"},
            rng=random.Random(1),
        )
        self.assertEqual(
            "To nie był czysty dom.",
            preview.source,
        )
        self.assertEqual(
            "これはきれいなうちじゃなかった",
            preview.answer,
        )
        self.assertEqual("copula", preview.dynamic_slot)
        self.assertEqual("past_negative_plain", preview.forms["copula"])
        self.assertEqual(
            "attributive_nonpast",
            preview.forms["adjective"],
        )

    def test_simple_kore_wa_pattern_uses_complete_adjective_form(self) -> None:
        expected = {
            "predicate_plain_nonpast": "これはあかい",
            "predicate_polite_nonpast": "これはあかいです",
            "predicate_plain_past_negative": "これはあかくなかった",
        }
        for form_name, answer in expected.items():
            with self.subTest(form=form_name):
                preview = self.project.build_preview(
                    "adjective_simple",
                    form_name=form_name,
                    word_choices={"adjective": "akai"},
                    rng=random.Random(1),
                )
                self.assertEqual(answer, preview.answer)

    def test_kono_is_literal_xml_and_not_an_entity_field(self) -> None:
        root = ET.parse(self.project.maps_path).getroot()
        answers = [
            (node.findtext("answer") or "")
            for node in root.findall(".//sentence_pattern")
        ]
        self.assertIn("この{subject}は{adjective.form}", answers)
        self.assertTrue(
            all("jp_demonstrative" not in answer for answer in answers)
        )
        with self.assertRaisesRegex(ProjectError, "Unknown entity field"):
            self.project.analyze_pattern(
                SentencePattern(
                    id="implicit_kono",
                    label="implicit_kono",
                    question="{subject.pl_nominative}",
                    answer="{subject.jp_demonstrative}",
                )
            )

    def test_reporting_verb_is_selected_by_shared_word_tag(self) -> None:
        preview = self.project.build_preview(
            "quoted_predicate",
            form_name="past_plain",
            word_choices={
                "predicate": "oishii",
                "reporting_verb": "omou",
            },
            entity_choices={"subject": "meal"},
            rng=random.Random(1),
        )
        self.assertEqual(
            "あのしょくじがおいしいとおもった",
            preview.answer,
        )

    def test_te_iru_uses_fixed_te_and_dynamic_auxiliary(self) -> None:
        expected = {
            "dictionary": "ほんをよんでいる",
            "past_plain": "ほんをよんでいた",
            "plain_negative": "ほんをよんでいない",
            "past_negative_plain": "ほんをよんでいなかった",
        }
        for form_name, answer in expected.items():
            with self.subTest(form=form_name):
                preview = self.project.build_preview(
                    "te_iru_object",
                    form_name=form_name,
                    word_choices={
                        "verb": "yomu",
                        "progressive_auxiliary": "iru",
                    },
                    entity_choices={"object": "book"},
                    rng=random.Random(1),
                )
                self.assertEqual(answer, preview.answer)
                self.assertEqual("te_form", preview.forms["verb"])
                self.assertEqual(
                    form_name,
                    preview.forms["progressive_auxiliary"],
                )

    def test_before_action_uses_fixed_dictionary_and_dynamic_main_verb(self) -> None:
        preview = self.project.build_preview(
            "before_action",
            form_name="dictionary",
            word_choices={"verb_before": "neru", "verb_main": "miru"},
            entity_choices={"object": "television"},
            rng=random.Random(1),
        )
        self.assertEqual("ねるまえにテレビをみる", preview.answer)
        self.assertEqual("dictionary", preview.forms["verb_before"])
        self.assertEqual("verb_main", preview.dynamic_slot)

    def test_destination_uses_xml_relation(self) -> None:
        preview = self.project.build_preview(
            "destination",
            form_name="dictionary",
            word_choices={"verb": "iku"},
            entity_choices={"destination": "school"},
            rng=random.Random(1),
        )
        self.assertEqual("がっこうへいく", preview.answer)

    def test_past_form_selects_past_context(self) -> None:
        preview = self.project.build_preview(
            "transitive_object",
            form_name="past_plain",
            word_choices={"verb": "yomu"},
            entity_choices={"object": "book"},
            rng=random.Random(1),
        )
        self.assertEqual("past", preview.context_pool)
        self.assertEqual("yesterday", preview.context_option)
        self.assertEqual(
            "Wczoraj czytałem / czytałam książkę.",
            preview.source,
        )
        self.assertEqual("きのう、ほんをよんだ", preview.answer)

    def test_context_prefix_pairs_stay_synchronized(self) -> None:
        expected = {
            "yesterday": ("Wczoraj ", "きのう、"),
            "last_week": ("W zeszłym tygodniu ", "せんしゅう、"),
            "recently": ("Ostatnio ", "さいきん、"),
        }
        previews = [
            self.project.build_preview(
                "transitive_object",
                form_name="past_plain",
                word_choices={"verb": "yomu"},
                entity_choices={"object": "book"},
                rng=random.Random(seed),
            )
            for seed in range(8)
        ]
        for preview in previews:
            polish, japanese = expected[preview.context_option]
            self.assertTrue(preview.source.startswith(polish))
            self.assertTrue(preview.answer.startswith(japanese))

    def test_future_ready_forms_remain_in_dictionaries(self) -> None:
        verbs = self.project.words["verbs"]
        adjectives = self.project.words["adjectives"]
        self.assertEqual("よんで", verbs["yomu"].forms["te_form"].value)
        self.assertEqual(
            "おおきければ",
            adjectives["ookii"].forms["conditional"].value,
        )
        self.assertTrue(all(len(word.forms) == 15 for word in verbs.values()))
        self.assertTrue(
            all(len(word.forms) == 15 for word in adjectives.values())
        )

    def test_pattern_rejects_multiple_dynamic_targets(self) -> None:
        pattern = SentencePattern(
            "invalid",
            "invalid",
            "{verb.prompt}",
            "{verb.form}{progressive_auxiliary.form}",
        )
        with self.assertRaisesRegex(ProjectError, "more than one dynamic"):
            self.project.analyze_pattern(pattern)

    def test_save_and_reload_round_trip(self) -> None:
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for source in PROJECT.parent.glob("*.xml"):
                (root / source.name).write_bytes(source.read_bytes())
            project = QuizProject.load(root / "quiz_project.xml")
            project.save()
            reloaded = QuizProject.load(root / "quiz_project.xml")
            self.assertEqual([], reloaded.validate())
            self.assertEqual(set(project.entities), set(reloaded.entities))
            self.assertEqual(set(project.patterns), set(reloaded.patterns))
            self.assertEqual(
                set(project.tag_profiles),
                set(reloaded.tag_profiles),
            )
            self.assertEqual(
                project.entities["motorcycle"].profiles,
                reloaded.entities["motorcycle"].profiles,
            )

    def test_relationship_is_optional_in_bare_pattern(self) -> None:
        cases = {
            "hanasu": "はなした",
            "taberu": "たべた",
            "iku": "いった",
        }
        for verb_id, answer in cases.items():
            with self.subTest(verb=verb_id):
                preview = self.project.build_preview(
                    "simple",
                    form_name="past_plain",
                    word_choices={"verb": verb_id},
                    rng=random.Random(1),
                )
                self.assertTrue(preview.answer.endswith(answer))
                self.assertEqual("past", preview.context_pool)
                self.assertEqual({}, preview.entities)

    def test_new_role_name_requires_only_xml_style_data(self) -> None:
        self.project.words["verbs"]["iku"].relations["goal"] = [
            "land_destination"
        ]
        pattern = SentencePattern(
            "custom_goal",
            "custom_goal",
            "{verb.prompt} do {goal.pl_genitive}.",
            "{goal}へ{verb.form}",
        )
        preview = self.project.build_preview(
            pattern.id,
            form_name="dictionary",
            word_choices={"verb": "iku"},
            entity_choices={"goal": "school"},
            pattern_override=pattern,
            rng=random.Random(1),
        )
        self.assertEqual("がっこうへいく", preview.answer)
        self.assertIn(
            "shared tag(s): land_destination",
            preview.trace(),
        )

    def test_attached_dictionary_inventory_is_complete(self) -> None:
        self.assertEqual(44, len(self.project.words["verbs"]))
        self.assertEqual(84, len(self.project.words["adjectives"]))
        self.assertEqual(31, len(self.project.entities))
        self.assertIn("shizuka", self.project.words["adjectives"])

    def test_only_functional_word_selector_tags_remain_on_verbs(self) -> None:
        tagged = {
            word.id: word.categories
            for word in self.project.words["verbs"].values()
            if word.categories
        }
        self.assertEqual(
            {
                "iru": ["progressive_auxiliary"],
                "iu": ["reporting"],
                "omou": ["reporting"],
            },
            tagged,
        )

    def test_motorcycle_inherits_tags_from_vehicle_profile(self) -> None:
        motorcycle = self.project.entities["motorcycle"]
        self.assertEqual(["vehicle"], motorcycle.profiles)
        self.assertEqual([], motorcycle.categories)
        effective = set(self.project.effective_entity_tags(motorcycle))
        self.assertTrue(
            {
                "age_new_old",
                "color",
                "purchasable",
                "rideable",
                "speed",
            }.issubset(effective)
        )

    def test_new_motorcycle_needs_only_one_profile_reference(self) -> None:
        preview = self.project.build_preview(
            "described_object_action",
            form_name="past_plain",
            word_choices={"verb": "kau", "adjective": "atarashii"},
            entity_choices={"object": "motorcycle"},
            rng=random.Random(1),
        )
        self.assertTrue(
            preview.answer.endswith("あたらしいバイクをかった")
        )

    def test_adjective_tags_describe_meaning_not_old_slot_lists(self) -> None:
        adjectives = self.project.words["adjectives"]
        self.assertEqual(["brightness"], adjectives["akarui"].categories)
        self.assertEqual(["brightness"], adjectives["kurai"].categories)
        self.assertEqual(
            ["appearance", "cleanliness"],
            adjectives["kirei"].categories,
        )
        self.assertEqual(
            ["cleanliness"],
            adjectives["kitanai"].categories,
        )
        self.assertEqual(["personality"], adjectives["majime"].categories)
        person_tags = set(
            self.project.effective_entity_tags("teacher")
        )
        self.assertIn("general_value", person_tags)
        self.assertIn("sound", person_tags)

    def test_relations_are_capabilities_not_broad_entity_classes(self) -> None:
        for entity_id in ("home", "school", "station"):
            preview = self.project.build_preview(
                "transitive_object",
                form_name="dictionary",
                word_choices={"verb": "tateru"},
                entity_choices={"object": entity_id},
                rng=random.Random(1),
            )
            self.assertTrue(preview.answer.endswith("をたてる"))
        for entity_id in ("book", "motorcycle", "ringo"):
            with self.subTest(entity=entity_id):
                with self.assertRaisesRegex(ProjectError, "not eligible"):
                    self.project.build_preview(
                        "transitive_object",
                        form_name="dictionary",
                        word_choices={"verb": "tateru"},
                        entity_choices={"object": entity_id},
                    )

    def test_future_vehicle_pattern_requires_no_python_change(self) -> None:
        pattern = SentencePattern(
            "ride_vehicle",
            "ride_vehicle",
            "{verb.prompt} {vehicle.pl_instrumental}.",
            "{vehicle}に{verb.form}",
        )
        preview = self.project.build_preview(
            pattern.id,
            form_name="dictionary",
            word_choices={"verb": "noru"},
            entity_choices={"vehicle": "motorcycle"},
            pattern_override=pattern,
            rng=random.Random(1),
        )
        self.assertEqual("バイクにのる", preview.answer)

    def test_attached_drink_and_song_examples_use_generic_relations(
        self,
    ) -> None:
        cases = (
            ("nomu", "water", "みずをのむ"),
            ("nomu", "tea", "おちゃをのむ"),
            ("nomu", "coffee", "コーヒーをのむ"),
            ("utau", "song", "うたをうたう"),
        )
        for verb_id, entity_id, expected in cases:
            with self.subTest(verb=verb_id, entity=entity_id):
                preview = self.project.build_preview(
                    "transitive_object",
                    form_name="dictionary",
                    word_choices={"verb": verb_id},
                    entity_choices={"object": entity_id},
                    rng=random.Random(1),
                )
                self.assertEqual(expected, preview.answer)

    def test_adjective_filter_is_optional(self) -> None:
        common = {
            "pattern_id": "described_object_action",
            "form_name": "dictionary",
            "word_choices": {"verb": "taberu"},
            "entity_choices": {"object": "ringo"},
        }
        any_stats = self.project.count_possibilities(**common)
        i_stats = self.project.count_possibilities(
            **common,
            adjective_type="i_adjective",
        )
        na_stats = self.project.count_possibilities(
            **common,
            adjective_type="na_adjective",
        )
        self.assertEqual(
            any_stats.word_counts["adjective"],
            i_stats.word_counts["adjective"]
            + na_stats.word_counts["adjective"],
        )

    def test_counter_reports_exact_metadata(self) -> None:
        stats = self.project.count_possibilities(
            "transitive_object",
            form_name="dictionary",
            word_choices={"verb": "taberu"},
        )
        edible_entities = {
            entity.id
            for entity in self.project.entities.values()
            if "edible" in self.project.effective_entity_tags(entity)
        }
        self.assertEqual(1, stats.word_counts["verb"])
        self.assertEqual(
            len(edible_entities),
            stats.entity_counts["object"],
        )
        self.assertEqual(
            len(edible_entities),
            stats.connection_counts["verb → object"],
        )
        self.assertEqual(
            stats.vocabulary_combinations * 3,
            stats.generated_sentences,
        )
        self.assertIn("Eligible items", stats.summary())


if __name__ == "__main__":
    unittest.main()
