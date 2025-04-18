#
# SC3020
# Project 2
# Pipesyntax Generation
#


# setup logging
import logging
import re

logging.basicConfig(level=logging.DEBUG)
log = logging.getLogger(__name__)


def gen_chunk(statements: list[str], cost: float = 0, in_sql: str = "") -> str:
    """Generate Pipeline SQL from given statements and cost.

    Args:
        statements: List of SQL statements to chain using "|>" pipeline operator.
        cost: Cost of the SQL statements.
        in_sql: SQL statement to prepend to the generated SQL chunk.
    Returns: Pipesyntax SQL in the format:
        [<in_sql>
        |> ]<STATEMENT 1>
        |> <STATEMENT 2>
        ...
        -- cost: <cost>
    """
    # chain in_sql with "|>" operator if it is not empty
    in_sql_pipe = "" if len(in_sql) == 0 else f"{in_sql}|> "
    return in_sql_pipe + "\n|> ".join(statements) + f"\n-- cost: {cost}\n"


def indent(sql: str, indent: int = 2) -> str:
    """Indent SQL statement with given indent.

    Args:
        sql: SQL statement to reindent.
        indent: Indent level.
    Returns:
        Indented SQL statement.
    """
    return "\n".join(" " * indent + line for line in sql.splitlines())


def join_clause(join_type: str) -> str:
    """Convert join type to SQL join clause.

    Args:
        join_type: Join type in QEP.
    Returns:
        Equivalent SQL join clause.
    """
    if join_type == "Inner":
        return "INNER JOIN"
    if join_type == "Left":
        return "LEFT JOIN"
    if join_type == "Right":
        return "RIGHT JOIN"
    if join_type == "Full":
        return "FULL OUTER JOIN"
    if join_type == "Anti":
        return "WHERE NOT EXISTS"
    if join_type == "Semi":
        return "WHERE EXISTS"

    raise ValueError(f"Unsupported join type: {join_type}")


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
            for i, plan in enumerate(node["Plans"]):
                self.register_subplan(plan)

    QUOTED_SUBPLAN_REGEX = re.compile(r"`\((SubPlan \d+)\)`")

    def resolve_subplan(self, expr: str) -> str:
        """Resolve subplan references in the given expression."""

        if match := re.search(self.QUOTED_SUBPLAN_REGEX, expr):
            subplan_name = match[1]
            # resolve subplan references to the actual sql
            if subplan_name in self.subplans["SubPlan"]:
                return re.sub(
                    self.QUOTED_SUBPLAN_REGEX,
                    self.subplans["SubPlan"][subplan_name],
                    expr,
                )
            log.warning(f"Subplan {subplan_name} not found")

        return expr

    def gen_projection(self, node: dict) -> str:
        """Generate projection as SELECT statement from QEP node."""
        # resolve subplan references in projected columns
        columns = [self.resolve_subplan(col) for col in node.get("Output", [])]
        if node.get("Strategy") == "Hashed" or node.get("Node Type") == "Unique":
            return f"SELECT DISTINCT {', '.join(columns)}"
        return f"SELECT {', '.join(columns)}"

    def gen_filters(self, node: dict) -> list[str]:
        """Generate filters from QEP node as WHERE statements

        Args:
            node: Preprocessed QEP node.
        Returns:
            List of WHERE statements generated from the filters.
        """
        if "Filters" not in node:
            return []
        filters = [self.resolve_subplan(f) for f in node["Filters"]]
        # filters already have parenthesis around them so precedence is already preserved
        # when joining them with 'AND'
        return [f"WHERE {' AND '.join(filters)}"]

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
        statements = (
            [
                f"FROM `{node['Relation Name']}` AS `{node['Alias']}`",
            ]
            + self.gen_filters(node)
            + [self.gen_projection(node)]
        )

        if node["Node Type"] in {"Index Only Scan", "Index Scan"}:
            # index scans read rows in the order of the index
            # reflect this by adding an ORDER BY
            direction = "ASC" if node["Scan Direction"] == "Forward" else "DESC"
            statements.append(f"ORDER BY {', '.join(node['Index Key'])} {direction}")
        return gen_chunk(statements, node["Total Cost"], in_sql)

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
        # qep lists grouping keys first but aggregate expects only aggregation expressions
        group_keys = set()
        if "Group Key" in node:
            for group_key in node["Group Key"]:
                group_keys.add(group_key)
                if "." in group_key:
                    # also match unqualified group keys
                    group_keys.add(group_key.split(".")[1])

        # skip the grouping keys when building aggregates
        aggregates = [o for o in node["Output"] if o not in group_keys]
        statements = [
            f"AGGREGATE {', '.join(aggregates)}{grouping}"
            # filters needed to implement to 'HAVING' filter on aggregation
        ] + self.gen_filters(node)
        return gen_chunk(statements, node["Total Cost"], in_sql)

    def gen_orderby(self, node: dict) -> str:
        """Generate SQL statements from given sort QEP node.

        Args:
            node: Preprocessed orderby QEP node.
        Returns:
            Generated SQL statements with cost.
        """
        # recursively generate sql in nested plans
        in_sql = "".join(self.gen_nested(node))
        statements = self.gen_filters(node) + [
            self.gen_projection(node),
            f"ORDER BY {', '.join(node['Sort Key'])}",
        ]

        return gen_chunk(statements, node["Total Cost"], in_sql)

    def gen_limit(self, node: dict) -> str:
        """Generate SQL statements from given limit QEP node.

        Args:
            node: Preprocessed limit QEP node.
        Returns:
            Generated SQL statements with cost.
        """
        # recursively generate sql in nested plans
        in_sql = "".join(self.gen_nested(node))

        statements = [
            f"LIMIT {node['Plan Rows']}",
            self.gen_projection(node),
        ]
        return gen_chunk(
            statements,
            node["Total Cost"],
            in_sql,
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
        if len(operands) > 2:
            logging.warning(f"Ignoring {len(operands)-2} operands, assuming Subplan.")

        lhs_sql, rhs_sql = operands[0], operands[1]

        # fetch aliases from child plans
        def alias(n):
            return f"|> AS `{n['Alias']}`" if "Alias" in n else ""

        lhs_alias = alias(node["Plans"][0])
        rhs_alias = alias(node["Plans"][1])

        # generate join statement
        join_on = (
            f" ON {self.resolve_subplan(node['Join On'])}" if "Join On" in node else ""
        )

        join_sql = f"""{lhs_sql + lhs_alias}
|> {join_clause(node['Join Type'])} (
{indent(rhs_sql+rhs_alias)}
){join_on}"""

        statements = [join_sql] + self.gen_filters(node) + [self.gen_projection(node)]
        return gen_chunk(statements, node["Total Cost"])

    def gen_initplans(self) -> str:
        """Generate registered initplans as a single WITH SQL statement"""
        initplans = self.subplans["InitPlan"]
        if len(initplans) == 0:
            return ""

        initplan_sqls = [f"`{name}` AS {sql}" for name, sql in initplans.items()]
        return f"WITH {', '.join(initplan_sqls)}\n"

    def generate(self, node: dict, top_level: bool = False) -> str:
        """Generate pipesyntax SQL statements from given preprocessed QEP node.
        Args:
            node: Preprocessed query execution plan node.
            top_level: Whether the node is a top level node.
        Returns:
            Pipesyntax SQL statements generated from the QEP node.
        """
        if "Parent Relationship" in node and node["Parent Relationship"] in [
            "InitPlan",
            "SubPlan",
        ]:
            # skip already registered subplans
            log.warning("Skipping node.", node)
            return ""

        if top_level:
            # include initplans in the top level qep node sql
            return self.gen_initplans() + self.generate(node)
        
        node_type = node.get("Node Type", "")
        if node_type == "Result":
            return self.gen_result(node)
        if node_type in ["Append", "SetOp"]:
            return self.gen_setop(node)
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

    def gen_setop(self, node: dict) -> str:
        """Generate SQL statements for set operations like UNION, UNION ALL, INTERSECT."""
        if "Plans" not in node or len(node["Plans"]) < 2:
            raise ValueError("At least 2 child plans requied for set operation")
        # generate SQL for each child subplan
        child_sql = [self.generate(plan) for plan in node["Plans"]]

        # determine type of the set operation 
        node_type = node["Node Type"]
        # default 
        operator = "UNION"
        if node_type == "Append":
            operator = "UNION ALL"
        elif node_type == "SetOp":
            command = node.get("Command", "UNION").upper()
            strategy = node.get("Strategy", "Sorted").upper()
            all_flag = node.get("All", False)

            if command == "INTERSECT":
                operator = "INTERSECT ALL" if all_flag else "INTERSECT"
            elif command == "EXCEPT":
                operator = "EXCEPT ALL" if all_flag else "EXCEPT"
            else:
                operator = "UNION ALL" if all_flag else "UNION"

            # Optionally note strategy in comment
            if strategy == "HASHED":
                operator += "  -- hashed"
        else:
            log.warning(f"Unrecognized set operation node type: {node_type}, using default UNION")

        combined = f"\n{operator}\n".join(child_sql)
        return f"{combined}\n-- cost: {node['Total Cost']}\n"
        
    def gen_result(self, node: dict) -> str:
        """Generate SQL for constant expressions, select"""
        if "Output" in node:
            columns = [self.resolve_subplan(c) for c in node["Output"]]
            return gen_chunk([f"SELECT {', '.join(columns)}"], node.get("Total Cost", 0))
        return gen_chunk(["SELECT NULL"], node.get("Total Cost", 0))
    
def generate(plan: dict) -> str:
    """Generate pipesyntax SQL from given preprocesed QEP plan.

    Traverses the plan preorder generating pipesyntax sql.

    Args:
        plan : Query execution plan
    Returns:
        str: Pipesyntax SQL generated from the plan
    """

    return PipeSyntax(plan).generate(plan, top_level=True)
