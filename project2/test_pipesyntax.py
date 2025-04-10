#
# SC3020
# Project 2
# Pipesyntax Generation Unit Tests
#

import pytest

from pipesyntax import PipeSyntax, generate
from preprocessing import Postgres, preprocess, pushup_aliases

SCAN_QEP = {
    "Node Type": "Index Only Scan",
    "Parent Relationship": "Outer",
    "Parallel Aware": False,
    "Async Capable": False,
    "Scan Direction": "Forward",
    "Index Name": "customer_pkey",
    "Relation Name": "customer",
    "Schema": "public",
    "Alias": "customer",
    "Startup Cost": 0.42,
    "Total Cost": 3937.42,
    "Plan Rows": 150000,
    "Plan Width": 4,
    "Output": ["customer.c_custkey"],
    "Index Key": ["c_custkey"],
}

SCAN_SQL = """FROM `public`.`customer` AS customer
|> SELECT customer.c_custkey
|> ORDER BY c_custkey ASC
-- cost: 3937.42
"""


@pytest.fixture
def query_plans(db: Postgres, query_sqls: list[str]):
    return [preprocess(sql, db) for sql in query_sqls]


def test_pipesyntax_generate_projection():
    pipesyntax = PipeSyntax()

    assert (
        pipesyntax.gen_projection(
            {
                "Output": [
                    "l_orderkey",
                    "MAX(A, B)",
                    "l_comment",
                ],
            }
        )
        == "SELECT l_orderkey, MAX(A, B), l_comment"
    )


def test_pipesyntax_generate_scan():
    pipesyntax = PipeSyntax()
    assert pipesyntax.gen_scan(SCAN_QEP) == SCAN_SQL


def test_pipesyntax_generate_aggregate():
    pipesyntax = PipeSyntax()
    assert (
        pipesyntax.gen_aggregate(
            {
                "Node Type": "Aggregate",
                "Strategy": "Sorted",
                "Partial Mode": "Simple",
                "Parallel Aware": False,
                "Async Capable": False,
                "Startup Cost": 0.42,
                "Total Cost": 9895.42,
                "Plan Rows": 150000,
                "Plan Width": 36,
                "Plans": [SCAN_QEP],
                "Output": ["c_custkey", "MIN(c_name)"],
                "Group Key": ["customer.c_custkey"],
            }
        )
        == SCAN_SQL
        + """|> AGGREGATE c_custkey, MIN(c_name) GROUP BY customer.c_custkey
-- cost: 9895.42
"""
    )


def test_pipesyntax_generate_orderby():
    pipesyntax = PipeSyntax()
    assert (
        pipesyntax.gen_orderby(
            {
                "Node Type": "Sort",
                "Startup Cost": 21208.45,
                "Total Cost": 21583.45,
                "Plan Rows": 150000,
                "Plan Width": 19,
                "Plans": [SCAN_QEP],
                "Output": ["c_name"],
                "Sort Key": ["customer.c_name", "customer.c_address DESC"],
            }
        )
        == SCAN_SQL
        + """|> ORDER BY customer.c_name, customer.c_address DESC
|> SELECT c_name
-- cost: 21583.45
"""
    )


def test_pipesyntax_generate_limit():
    pipesyntax = PipeSyntax()
    assert (
        pipesyntax.gen_limit(
            {
                "Node Type": "Limit",
                "Parallel Aware": False,
                "Async Capable": False,
                "Startup Cost": 63423.31,
                "Total Cost": 63423.32,
                "Plan Rows": 1000,
                "Plan Width": 270,
                "Output": [
                    "supplier.s_acctbal",
                    "supplier.s_name",
                    "nation.n_name",
                ],
                "Plans": [SCAN_QEP],
            }
        )
        == SCAN_SQL
        + """|> LIMIT 1000
|> SELECT supplier.s_acctbal, supplier.s_name, nation.n_name
-- cost: 63423.32
"""
    )


def test_pipesyntax_generate_join():
    pipesyntax = PipeSyntax()

    assert (
        (
            pipesyntax.gen_join(
                {
                    "Node Type": "Hash Join",
                    "Parallel Aware": False,
                    "Async Capable": False,
                    "Join Type": "Inner",
                    "Startup Cost": 5812.42,
                    "Total Cost": 11442.18,
                    "Plan Rows": 150000,
                    "Plan Width": 19,
                    "Output": ["c1.c_name"],
                    "Inner Unique": True,
                    "Join On": "(c1.c_custkey = c2.c_custkey)",
                    "Plans": [
                        {
                            "Node Type": "Seq Scan",
                            "Parent Relationship": "Outer",
                            "Parallel Aware": False,
                            "Async Capable": False,
                            "Relation Name": "customer",
                            "Schema": "public",
                            "Alias": "c1",
                            "Startup Cost": 0.00,
                            "Total Cost": 5236.00,
                            "Plan Rows": 150000,
                            "Plan Width": 23,
                            "Output": [
                                "c1.c_custkey",
                                "c1.c_name",
                            ],
                        },
                        {
                            "Node Type": "Hash",
                            "Parent Relationship": "Inner",
                            "Parallel Aware": False,
                            "Async Capable": False,
                            "Startup Cost": 3937.42,
                            "Total Cost": 3937.42,
                            "Plan Rows": 150000,
                            "Plan Width": 4,
                            "Alias": "c2",
                            "Output": ["c2.c_custkey"],
                            "Plans": [
                                {
                                    "Node Type": "Index Only Scan",
                                    "Parent Relationship": "Outer",
                                    "Parallel Aware": False,
                                    "Async Capable": False,
                                    "Scan Direction": "Forward",
                                    "Index Name": "customer_pkey",
                                    "Index Key": ["c_custkey"],
                                    "Relation Name": "customer",
                                    "Schema": "public",
                                    "Alias": "c2",
                                    "Startup Cost": 0.42,
                                    "Total Cost": 3937.42,
                                    "Plan Rows": 150000,
                                    "Plan Width": 4,
                                    "Output": ["c2.c_custkey"],
                                }
                            ],
                        },
                    ],
                }
            )
        )
        == """(
  FROM `public`.`customer` AS c1
  |> SELECT c1.c_custkey, c1.c_name
  -- cost: 5236.0
) AS `c1` INNER JOIN (
  FROM `public`.`customer` AS c2
  |> SELECT c2.c_custkey
  |> ORDER BY c_custkey ASC
  -- cost: 3937.42
) AS `c2` ON (c1.c_custkey = c2.c_custkey)
|> SELECT c1.c_name
-- cost: 11442.18
"""
    )


def test_generate(query_plans):
    """Test pipesyntax generation from query plans."""
    for i, plan in enumerate(query_plans):
        import json

        with open(f"plans/{i}.json", "w") as f:
            json.dump(generate(plan), f)
