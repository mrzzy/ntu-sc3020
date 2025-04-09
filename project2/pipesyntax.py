from typing import Any, Dict, List, Optional

from sqlglot import exp, parse_one

from preprocessing import Postgres, preprocess

# Global variable to store aggregate expressions
global_agg_expressions = []


def collect_plans(qep_node: Dict, plan_dict: Dict[str, Dict]) -> None:

    # Store subplan or initplan
    if qep_node.get("Parent Relationship") in ["InitPlan", "SubPlan"]:
        plan_name = qep_node.get("Subplan Name", "Unknown")
        plan_dict[plan_name] = qep_node

    # Continue traversing
    for child in qep_node.get("Plans", []):
        collect_plans(child, plan_dict)


def get_max_depth(plan: Dict) -> int:
    """
    Helper function
    Calculate the maximum depth of the plan tree
    """
    if not plan:
        return 0
    children = plan.get("Plans", [])
    if not children:
        return 0
    return 1 + max(get_max_depth(child) for child in children)


def parse_plan(
    plan: Dict,
    depth: int = 0,
    max_depth: Optional[int] = None,
    plan_dict: Dict[str, Dict] = {},
) -> str:
    if not plan:
        return ""
    if max_depth is None:
        max_depth = get_max_depth(plan)
    if plan_dict is None:
        plan_dict = {}

    node_type = plan.get("Node Type", "Unknown")
    children = plan.get("Plans", [])
    startup_cost = plan.get("Startup Cost", "N/A")
    total_cost = plan.get("Total Cost", "N/A")
    cost_display = f"{startup_cost} -> {total_cost}"
    # indent = "  " * (max_depth - depth)
    indent = "  " * (max_depth - depth - 1 if max_depth - depth - 1 >= 0 else 0)

    # Skip Hash node
    # if node_type == "Hash":
    #     return ""
    if node_type == "Hash":
        # child_results = []
        # for child in children:
        #     child_result = parse_plan(child, depth + 1, max_depth, plan_dict)
        #     if child_result:
        #         child_results.append(child_result)
        # return "\n".join(child_results) if child_results else ""
        return "\n".join(
            filter(
                None,
                [
                    parse_plan(child, depth + 1, max_depth, plan_dict)
                    for child in children
                ],
            )
        )

    # Initialize result string
    result = ""
    current_ops = ""

    # Handle FROM clause for scan operations
    if node_type in ("Seq Scan", "Index Scan", "Index Only Scan"):
        table = plan.get("Relation Name", plan.get("Alias", "Unknown"))
        alias = plan.get("Alias", table)

        # table scan
        if alias != table:
            current_ops += f"{indent}FROM {table} AS {alias} -- Cost: {cost_display}\n"
        else:
            current_ops += f"{indent}FROM {table} -- Cost: {cost_display}\n"

        # Handle WHERE
        conditions = [
            clean_expression(plan.get(k))
            for k in ["Filter", "Index Cond", "Recheck Cond"]
            if k in plan
        ]
        if conditions:
            current_ops += (
                f"{indent}|> WHERE {' AND '.join(conditions)} -- Cost: {cost_display}\n"
            )
        if node_type in ("Index Scan", "Index Only Scan") and "Primary Key" in plan:
            current_ops += (
                f"{indent}|> ORDER BY {plan['Primary Key']} -- Cost: {cost_display}\n"
            )

    elif "Filter" in plan:
        current_ops += f"{indent}|> WHERE {clean_expression(plan['Filter'])} -- Cost: {cost_display}\n"

    # Handle other node types
    if node_type == "Aggregate":
        projections = plan.get("Projections", [])
        group_keys = plan.get("Group Key", [])
        output_expr = format_projections(projections or global_agg_expressions)
        group_by = (
            f" GROUP BY {', '.join(clean_expression(k) for k in group_keys)}"
            if group_keys
            else ""
        )
        mode = plan.get("Partial Mode", "")
        label = "AGGREGATE" + (f" {mode.upper()}" if mode else "")
        current_ops += (
            f"{indent}|> {label} {output_expr}{group_by} -- Cost: {cost_display}\n"
        )

        # Format aggregate output
        if projections:
            output_expr = format_projections(projections)
        elif global_agg_expressions:
            output_expr = ", ".join(global_agg_expressions)
        else:
            output_expr = "aggregate values"

    elif node_type == "Gather":
        current_ops += f"{indent}|> GATHER -- Cost: {cost_display}\n"

    elif node_type == "Limit":
        current_ops += (
            f"{indent}|> LIMIT {plan.get('Plan Rows', 1)} -- Cost: {cost_display}\n"
        )

    elif node_type in ("Sort", "Incremental Sort"):
        keys = plan.get("Sort Key", [])
        pre_keys = plan.get("Presorted Key", [])
        cleaned_keys = [clean_expression(k) for k in keys]
        if node_type == "Incremental Sort" and pre_keys:
            cleaned_pre = [clean_expression(k) for k in pre_keys]
            current_ops += f"{indent}|> INCREMENTAL SORT (presorted: {', '.join(cleaned_pre)}) BY {', '.join(cleaned_keys)} -- Cost: {cost_display}\n"
        else:
            current_ops += f"{indent}|> ORDER BY {', '.join(cleaned_keys)} -- Cost: {cost_display}\n"

    elif node_type in ("Nested Loop", "Merge Join", "Hash Join"):
        cond_key = {
            "Nested Loop": "Join Filter",
            "Merge Join": "Merge Cond",
            "Hash Join": "Hash Cond",
        }.get(node_type, "")
        cond = clean_expression(plan.get(cond_key, ""))
        join_type = plan.get("Join Type", "Inner").upper()
        join_map = {
            "INNER": "",
            "LEFT": "LEFT OUTER ",
            "RIGHT": "RIGHT OUTER ",
            "FULL": "FULL OUTER ",
            "SEMI": "SEMI ",
            "ANTI": "ANTI ",
        }
        prefix = join_map.get(join_type, "")
        current_ops += (
            f"{indent}|> {prefix}{node_type.upper()}"
            + (f" ON {cond}" if cond else "")
            + f" -- Cost: {cost_display}\n"
        )

    elif node_type == "Materialize":
        current_ops += f"{indent}|> MATERIALIZE -- Cost: {cost_display}\n"

    # Handle subplan/initplan
    if plan.get("Parent Relationship") in ["SubPlan", "InitPlan"]:
        current_ops = (
            f"{indent}|> {plan.get('Subplan Name', 'Unnamed Plan')} -- Cost: {cost_display}\n"
            + current_ops
        )

    for child in children:
        result += parse_plan(child, depth + 1, max_depth, plan_dict) + "\n"
    result += current_ops

    return result.rstrip()


def clean_expression(expr) -> str:
    if not expr:  # Guard against None
        return ""

    if isinstance(expr, str):
        return (
            expr.replace("::text", "")
            .replace("::bpchar", "")
            .replace("::date", "")
            .replace("::timestamp without time zone", "")
            .replace("::numeric", "")
            .replace("~~", "LIKE")
        )
    return str(expr)


def get_aggregate_expressions(sql: str) -> List[str]:
    """
    Extract aggregate expressions from a SQL query using sqlglot.

    Args:
        sql: The SQL query string

    Returns:
        A list of aggregate expressions from the SQL query
    """
    try:
        # Parse the SQL query
        ast = parse_one(sql, dialect="postgres")

        # Find the top-level SELECT
        selects = list(ast.find_all(exp.Select))
        if not selects:
            return []

        # Get the expressions from the SELECT statement
        expressions = selects[0].expressions
        if not expressions:
            return []

        return [str(expr) for expr in expressions]

    except Exception as e:
        print(f"Error parsing SQL: {e}")
        return []


def format_projections(projections: List[str]) -> str:
    return ", ".join(clean_expression(p) for p in projections) if projections else "*"


def convert_qep_to_pipe_syntax(
    qep_plan: Dict[str, Any], agg_expressions: List[str] = []
) -> str:
    """
    Convert a Query Execution Plan to pipe syntax.

    Args:
        qep_plan: The Query Execution Plan from preprocessing.preprocess()
                 This is already a Python dictionary, not a JSON string

    Returns:
        A string representation of the plan in pipe syntax
    """
    if not qep_plan:
        return ""
    # First pass: collect all subplans and initplans
    plan_dict = {}
    collect_plans(qep_plan, plan_dict)
    # Store aggregate expressions for use in parse_plan
    global global_agg_expressions
    global_agg_expressions = agg_expressions or []
    # Second pass
    result = parse_plan(qep_plan, plan_dict=plan_dict)
    result = result.replace(" : ->", " ->")

    return result


def main(preprocessed_plan: Dict[str, Any], sql: str = "") -> str:
    if not preprocessed_plan:
        return ""

    # If SQL is provided, extract aggregate expressions
    agg_expressions = []
    if sql:
        agg_expressions = get_aggregate_expressions(sql)

    # Pass aggregate expressions to convert_qep_to_pipe_syntax
    return convert_qep_to_pipe_syntax(preprocessed_plan, agg_expressions)


if __name__ == "__main__":
    # print("***************")
    # try:
    #     with open("test.json") as f:
    #         json_str = f.read()

    #     if not json_str:
    #         print("Warning: Read empty file")

    #     result = convert_qep_to_pipe_syntax(json_str)
    #     print(result)

    # except Exception as e:
    #     print(f"Error in main: {e}")
    # print("***************")

    import os

    print("Testing pipe syntax generation")

    # test
    # sql = "SELECT * FROM customer WHERE c_custkey = 1"
    sql = ""

    try:
        # Connect to PostgreSQL
        db = Postgres(
            host="localhost",
            user="postgres",
            password=os.environ.get("POSTGRES_PASSWORD", "postgres"),
        )

        sql = """select
        l_returnflag,
        l_linestatus,
        sum(l_quantity) as sum_qty,
        sum(l_extendedprice) as sum_base_price,
        sum(l_extendedprice * (1 - l_discount)) as sum_disc_price,
        sum(l_extendedprice * (1 - l_discount) * (1 + l_tax)) as sum_charge,
        avg(l_quantity) as avg_qty,
        avg(l_extendedprice) as avg_price,
        avg(l_discount) as avg_disc,
        count(*) as count_order
        from
        lineitem
        where
        l_shipdate <= date '1998-12-01' - interval '108' day
        group by
        l_returnflag,
        l_linestatus
        order by
        l_returnflag,
        l_linestatus
        LIMIT 1;"""

        # Preprocess the SQL query
        qep_plan = preprocess(sql, db)
        print("[DEBUG] Preprocessed QEP:")
        print(json.dumps(qep_plan, indent=2))
        # Convert to pipe syntax
        print("\n[DEBUG] Pipe syntax output:")
        result = main(qep_plan, sql)
        print(result)

    except Exception as e:
        print(f"Error: {e}")
