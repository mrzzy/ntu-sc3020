#
# SC3020
# Project 2
# Pipesyntax Generation
#


# setup logging
import logging
import re
from dataclasses import dataclass

from preprocessing import SUBPLAN_REGEX, apply

logging.basicConfig(level=logging.DEBUG)
log = logging.getLogger(__name__)


def gen_chunk(statements: list[str], cost: float = 0) -> str:
    """Generate Pipeline SQL from given statements and cost.

    Args:
        statements: List of SQL statements to chain using "|>" pipeline operator.
        cost: Cost of the SQL statements.
    Returns: Pipesyntax SQL in the format:
        <STATEMENT 1>
        |> <STATEMENT 2>
        ...
        -- cost: <cost>
    """
    return "\n|> ".join(statements) + f"\n-- cost: {cost}\n"


def append(in_sql: str, next_sql: str):
    """Appends next_sql to in_sql with '|>' pipeline operator.

    Args:
        in_sql: SQL statement to append to.
        next_sql: SQL statement to append.

    Returns:
        If in_sql is empty, returns next_sql without pipeline operator.
        If next_sql is empty, returns in_sql without pipeline operator.
    """
    if not in_sql:
        return next_sql
    if not next_sql:
        return in_sql
    return in_sql + "|> " + next_sql


def indent(sql: str, indent: int = 2) -> str:
    """Indent SQL statement with given indent.

    Args:
        sql: SQL statement to reindent.
        indent: Indent level.
    Returns:
        Indented SQL statement.
    """
    return "\n".join(" " * indent + line for line in sql.splitlines())


class PipeSyntax:
    """Pipesyntax SQL Generator generates SQL from QEP."""

    def __init__(self, plan: dict = {}):
        self.subplans = {
            "InitPlan": {},
            "SubPlan": {},
        }
        self.register_subplan(plan)

    def register_subplan(self, node: dict):
        """Register subplans in the given QEP node."""
        if "Parent Relationship" in node and node["Parent Relationship"] in [
            "InitPlan",
            "SubPlan",
        ]:
            # recursively generate inner sql of subplan
            inner = node.copy()
            inner["Parent Relationship"] = ""
            inner_sql = gen_chunk(
                [f"{self.generate(inner)}|> {self.gen_projection(inner)}"],
                inner["Total Cost"],
            )
            self.subplans[node["Parent Relationship"]][
                node["Subplan Name"]
            ] = f"(\n{indent(inner_sql)}\n)"

        if "Plans" in node:
            for plan in node["Plans"]:
                self.register_subplan(plan)

    def resolve_subplan(self, expr: str) -> str:
        """Resolve subplan references in the given expression."""

        if match := re.search(SUBPLAN_REGEX, expr):
            subplan_name = match[1]
            # resolve subplan references to the actual sql
            if subplan_name in self.subplans["SubPlan"]:
                return re.sub(
                    SUBPLAN_REGEX, self.subplans["SubPlan"][subplan_name], expr
                )
            log.warning(f"Subplan {subplan_name} not found")

        return expr

    def gen_projection(self, node: dict) -> str:
        """Generate projection as SELECT statement from QEP node."""
        # resolve subplan references in projected columns
        columns = [self.resolve_subplan(c) for c in node["Output"]]
        return f"SELECT {', '.join(columns)}"

    def gen_scan(self, node: dict) -> str:
        """Generate SQL statements from given scan QEP node.

        Args:
            node: Preprocessed scan QEP node.
        Returns:
            Generated SQL statement with cost.
        """

        if node["Node Type"] == "Bitmap Index Scan":
            # bitmap index scans only reduces rows for a bitmap heap scan
            # which will recheck the filter condition on actual rows
            # we can safely ignore when generating functionally equivalent pipeline sql
            return ""

        # recursively generate sql in nested plans
        in_sql = "".join(self.gen_nested(node))

        # generate statements for scan
        schema = f"`{node['Schema']}`." if "Schema" in node else ""
        statements = [
            f"FROM {schema}`{node['Relation Name']}` AS {node['Alias']}",
            self.gen_projection(node),
        ]

        if node["Node Type"] in {"Index Only Scan", "Index Scan"}:
            # index scans read rows in the order of the index
            # reflect this by adding an ORDER BY
            direction = "ASC" if node["Scan Direction"] == "Forward" else "DESC"
            statements.append(f"ORDER BY {', '.join(node['Index Key'])} {direction}")
        return append(in_sql, gen_chunk(statements, node["Total Cost"]))

    def gen_aggregate(self, node: dict) -> str:
        """Generate SQL statements from given aggregate QEP node.

        Args:
            node: Preprocessed aggregate QEP node.
        Returns:
            Generated SQL statements with cost.
        """
        # recursively generate sql in nested plans
        in_sql = "".join(self.gen_nested(node))

        grouping = (
            f" GROUP BY {', '.join(node['Group Key'])}" if "Group Key" in node else ""
        )
        return append(
            in_sql,
            gen_chunk(
                [f"AGGREGATE {', '.join(node['Output'])}{grouping}"],
                node["Total Cost"],
            ),
        )

    def gen_orderby(self, node: dict) -> str:
        """Generate SQL statements from given orderby QEP node.

        Args:
            node: Preprocessed orderby QEP node.
        Returns:
            Generated SQL statements with cost.
        """
        # recursively generate sql in nested plans
        in_sql = "".join(self.gen_nested(node))
        return append(
            in_sql,
            gen_chunk(
                [
                    f"ORDER BY {', '.join(node['Sort Key'])}",
                    self.gen_projection(node),
                ],
                node["Total Cost"],
            ),
        )

    def gen_limit(self, node: dict) -> str:
        """Generate SQL statements from given limit QEP node.

        Args:
            node: Preprocessed limit QEP node.
        Returns:
            Generated SQL statements with cost.
        """
        # recursively generate sql in nested plans
        in_sql = "".join(self.gen_nested(node))

        return append(
            in_sql,
            gen_chunk(
                [f"LIMIT {node['Plan Rows']}", self.gen_projection(node)],
                node["Total Cost"],
            ),
        )

    def gen_join(self, node: dict) -> str:
        """Generate SQL statements from given join QEP node.

        Args:
            node: Preprocessed join QEP node.
        Returns:
            Generated SQL statements with cost.
        """

        # recursively generate sql in nested plans
        operands = self.gen_nested(node)
        if len(operands) < 2:
            raise ValueError(f"Expected >= 2 operands, got: {len(operands)}")

        # generate join statement
        rhs_sql = operands.pop()
        lhs_sql = operands.pop()
        in_sql = "".join(operands)

        alias = lambda n: f" AS `{n['Alias']}`" if "Alias" in n else ""
        lhs_alias = alias(node["Plans"][-2])
        rhs_alias = alias(node["Plans"][-1])

        join_on = (
            f" ON {self.resolve_subplan(node['Join On'])}" if "Join On" in node else ""
        )

        join_sql = f"""{in_sql}(
{indent(lhs_sql)}
){lhs_alias} {node['Join Type'].upper()} JOIN (
{indent(rhs_sql)}
){rhs_alias}{join_on}"""

        return gen_chunk([join_sql, self.gen_projection(node)], node["Total Cost"])

    def generate(self, node: dict) -> str:
        """Generate pipesyntax SQL statements from given preprocessed QEP node.
        Args:
            node: Preprocessed query execution plan node.
        Returns:
            Pipesyntax SQL statements generated from the QEP node.
        """

        if "Relation Name" in node:
            return self.gen_scan(node)
        if node["Node Type"] in ["HashAggregate", "Aggregate", "Group"]:
            return self.gen_aggregate(node)
        if "Sort Key" in node:
            return self.gen_orderby(node)
        if "Join Type" in node:
            return self.gen_join(node)
        if node["Node Type"] == "Limit":
            return self.gen_limit(node)
        if node["Parent Relationship"] in ["InitPlan", "SubPlan"]:
            # skip already registered subplans
            return ""

        # unknown qep node: ignore node and generate from nested qep nodes
        log.warning(f"Ignoring node: {node['Node Type']}")
        return "".join(self.gen_nested(node))

    def gen_nested(self, node: dict) -> list[str]:
        """Generate SQL statements from nested plans in gven QEP node.
        Args:
            node: Preprocessed QEP node.
        Returns:
            List of Generated SQL statements.
        """
        if "Plans" not in node:
            return []

        return [self.generate(plan) for plan in node["Plans"]]


def generate(plan: dict) -> str:
    """Generate pipesyntax SQL from given preprocesed QEP plan.

    Traverses the plan preorder generating pipesyntax sql.

    Args:
        plan : Query execution plan
    Returns:
        str: Pipesyntax SQL generated from the plan
    """

    return PipeSyntax(plan).generate(plan)
