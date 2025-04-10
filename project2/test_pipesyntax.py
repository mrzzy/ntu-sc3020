#
# SC3020
# Project 2
# Pipesyntax Generation Unit Tests
#

import pytest

from pipesyntax import PipeSyntax, generate
from preprocessing import Postgres, preprocess


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
    chunk = pipesyntax.gen_scan(
        {
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
    )
    assert chunk.statements == [
        "FROM `public`.`customer` AS customer",
        "SELECT customer.c_custkey",
        "ORDER BY c_custkey ASC",
    ]
    assert chunk.cost == pytest.approx(3937.42)


def test_pipesyntax_generate_aggregate():
    pipesyntax = PipeSyntax()
    chunk = pipesyntax.gen_aggregate(
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
            "Plans": [],
            "Output": ["c_custkey", "MIN(c_name)"],
            "Group Key": ["customer.c_custkey"],
        }
    )
    assert chunk.statements == [
        "AGGREGATE c_custkey, MIN(c_name) GROUP BY customer.c_custkey",
    ]
    assert chunk.cost == pytest.approx(9895.42)


def test_generate(query_plans):
    """Test pipesyntax generation from query plans."""
    for plan in query_plans:
        generate(plan)
