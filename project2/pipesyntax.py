#
# SC3020
# Project 2
# Pipesyntax Generation
#


from preprocessing import apply


class PipeSyntax:
    """Pipesyntax SQL Generator generates SQL from QEP."""

    def __init__(self):
        self.subplans = {}

    def resolve_subplan(self, expr: str) -> str:
        """Resolve subplan references in the given expression."""
        # TODO: resolve subplan references
        return expr

    def gen_projection(self, node: dict) -> str:
        """Generate projection as SELECT statement from QEP node."""
        # resolve subplan references in projected columns
        columns = [self.resolve_subplan(c) for c in node["Output"]]
        return f"SELECT {', '.join(columns)}"

    def gen_scan(self, node: dict) -> list[str]:
        """Generate SQL statements from given scan QEP node."""
        if node["Node Type"] == "Bitmap Index Scan":
            # bitmap index scans only reduces rows for a bitmap heap scan
            # which will recheck the filter condition on actual rows
            # we can safely ignore when generating functionally equivalent pipeline sql
            return []

        statements = [
            f"FROM `{node['Schema']}`.`{node['Relation Name']}` AS {node['Alias']}",
            self.gen_projection(node),
        ]

        if node["Node Type"] in {"Index Only Scan", "Index Scan"}:
            # index scans read rows in the order of the index
            # reflect this by adding an ORDER BY
            direction = "ASC" if node["Scan Direction"] == "Forward" else "DESC"
            statements.append(f"ORDER BY {', '.join(node['Index Key'])} {direction}")
        return statements

    def generate(self, node: dict) -> list[str]:
        """Generate pipesyntax SQL statements from given preprocessed QEP node.
        Args:
            node: Preprocessed query execution plan node.
        Returns:
            Pipesyntax SQL statements generated from the QEP node.
        """

        if "Relation Name" in node:
            return self.gen_scan(node)


def generate(plan: dict) -> str:
    """Generate pipesyntax SQL from given preprocesed QEP plan.

    Traverses the plan preorder generating pipesyntax sql.

    Args:
        plan : Query execution plan
    Returns:
        str: Pipesyntax SQL generated from the plan
    """

    pipesyntax = PipeSyntax()
