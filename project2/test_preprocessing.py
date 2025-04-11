#
# SC3020
# Project 2
# Preprocessing Unit Tests
#

from copy import deepcopy

from preprocessing import (
    EXPR_LIST_KEYS,
    EXPR_SINGLE_KEYS,
    CTETransformer,
    DialectTransformer,
    FilterTransformer,
    IndexKeyTransformer,
    JoinKeyTransformer,
    Postgres,
    SubplanNameTransformer,
    apply,
    correct_sql_arrays,
    preprocess,
    pushup_aliases,
    transform,
)


def collect_expr(exprs: list[str]):
    def collect(qep_node: dict, depth: int, subplan: str):
        for key in qep_node.keys():
            if key in EXPR_LIST_KEYS:
                exprs.extend(qep_node[key])
            if key in EXPR_SINGLE_KEYS:
                exprs.append(qep_node[key])
        return qep_node

    return collect


def test_correct_arrays():
    test_cases = [
        (
            "CAST('{16,18,47,11,1,42,10,27}' AS ARRAY<INT64>)",
            "[CAST(16 AS INT64), CAST(18 AS INT64), CAST(47 AS INT64), CAST(11 AS INT64), CAST(1 AS INT64), CAST(42 AS INT64), CAST(10 AS INT64), CAST(27 AS INT64)]",
        ),
        (
            "CAST('{a,b,c}' AS ARRAY<VARCHAR>)",
            "[CAST(a AS VARCHAR), CAST(b AS VARCHAR), CAST(c AS VARCHAR)]",
        ),
    ]

    for expr, expected in test_cases:
        assert correct_sql_arrays(expr) == expected


def test_postgres_explain(db: Postgres, query_sqls: list[str]):
    # run explain on each query in queries
    for sql in query_sqls:
        db.explain(sql)


def test_postgres_get_index_key(db: Postgres):
    assert db.get_index_key("idx_lineitem_part_supp") == ["l_partkey", "l_suppkey"]


def test_cte_transform(db: Postgres, query_sqls: list[str]):
    # test: TPC-H 15th query 15.sql
    plan = db.explain(query_sqls[15 - 1])
    plan = transform(plan, [CTETransformer()])

    nodes = []

    def collect_cte(qep_node: dict, depth: int, subplan: str):
        if qep_node["Node Type"] == "CTE Scan" and "Relation Name" in qep_node:
            nodes.append(qep_node)
        return qep_node

    apply(plan, collect_cte)

    assert len(nodes) == 2
    assert all(n["Relation Name"] == n["CTE Name"] for n in nodes)


def test_join_key_transform(db: Postgres, query_sqls: list[str]):
    # test: TPC-H 13th query 13.sql
    plan = db.explain(query_sqls[13 - 1])
    plan = transform(plan, [JoinKeyTransformer()])

    join_keys = []

    def collect_join(qep_node: dict, depth: int, subplan: str):
        if "Join On" in qep_node:
            join_keys.append(qep_node["Join On"])
        return qep_node

    apply(plan, collect_join)

    assert len(join_keys) == 1
    assert join_keys[0] == "(orders.o_custkey = customer.c_custkey)"


def test_filter_transform(db: Postgres, query_sqls: list[str]):
    # test: TPC-H 1th query 1.sql
    plan = db.explain(query_sqls[1 - 1])
    plan = transform(plan, [FilterTransformer()])

    filters = []

    def collect_filter(qep_node: dict, depth: int, subplan: str):
        if "Filters" in qep_node:
            filters.append(qep_node["Filters"])
        return qep_node

    apply(plan, collect_filter)

    assert len(filters) == 1
    assert filters[0] == [
        "(lineitem.l_shipdate <= '1998-08-15 00:00:00'::timestamp without time zone)"
    ]


def test_subplan_name_transform(db: Postgres, query_sqls: list[str]):
    # test: TPC-H 15th query 15.sql
    plan = db.explain(query_sqls[15 - 1])
    plan = transform(plan, [SubplanNameTransformer()])

    def check_names(qep_node: dict, depth: int, subplan: str):
        if "Subplan Name" in qep_node:
            assert "CTE " not in qep_node["Subplan Name"]
        return qep_node

    apply(plan, check_names)


def test_dialect_transform(db: Postgres, query_sqls: list[str]):
    # test: TPC-H 16th query 16.sql
    plan = db.explain(query_sqls[16 - 1])
    new_plan = transform(deepcopy(plan), [DialectTransformer()])

    def collect_to(exprs: list[str]):
        def collect_expr(qep_node: dict, depth: int):
            for key in qep_node.keys():
                if key in EXPR_LIST_KEYS:
                    exprs.extend(qep_node[key])
                if key in EXPR_SINGLE_KEYS:
                    exprs.append(qep_node[key])
            return qep_node

        return collect_expr

    new_plan = transform(deepcopy(plan), [DialectTransformer()])

    old_exprs, exprs = [], []
    apply(plan, collect_expr(old_exprs))
    apply(new_plan, collect_expr(exprs))

    assert len(exprs) == 46
    assert (
        old_exprs[34] == "(NOT (ANY (partsupp.ps_suppkey = (hashed SubPlan 1).col1)))"
    )
    assert exprs[34] == "(NOT ((partsupp.ps_suppkey) IN (`(SubPlan 1)`)))"
    assert (
        old_exprs[45]
        == "((part.p_brand <> 'Brand#53'::bpchar) AND ((part.p_type)::text !~~ 'SMALL POLISHED%'::text) AND (part.p_size = ANY ('{16,18,47,11,1,42,10,27}'::integer[])))"
    )
    assert (
        exprs[45]
        == "((part.p_brand <> CAST('Brand#53' AS STRING)) AND (NOT CAST((part.p_type) AS STRING) LIKE CAST('SMALL POLISHED%' AS STRING)) AND (part.p_size IN ([CAST(16 AS INT64), CAST(18 AS INT64), CAST(47 AS INT64), CAST(11 AS INT64), CAST(1 AS INT64), CAST(42 AS INT64), CAST(10 AS INT64), CAST(27 AS INT64)])))"
    )


def test_pushup_alias(db: Postgres, query_sqls: list[str]):
    plan = pushup_aliases(
        {
            "Plans": [
                {
                    "Alias": "a",
                    "Plans": [{"Alias": "b"}],
                }
            ]
        }
    )

    assert plan["Alias"] == "a"
    assert plan["Plans"][0]["Alias"] == "a"


def test_preprocess(db: Postgres, query_sqls: list[str]):
    for sql in query_sqls:
        preprocess(sql, db)
