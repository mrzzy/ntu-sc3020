import json
import re

def get_max_depth(plan):
    """helper function"""
    if not plan:
        return 0
    children = plan.get("Plans", [])
    if not children:
        return 0
    return 1 + max(get_max_depth(child) for child in children)

def parse_plan(plan, depth=0, max_depth=None):
    if not plan:
        return ""
    if max_depth is None:
        max_depth = get_max_depth(plan)
        depth = 0

    node_type = plan.get("Node Type", "Unknown")
    children = plan.get("Plans", [])
    startup_cost = plan.get("Startup Cost", "N/A")
    total_cost = plan.get("Total Cost", "N/A")
    cost_display = f"{startup_cost} : -> {total_cost}"  # Custom format
    indent = "  " * (max_depth - depth)
    # Skip Hash node
    if node_type == "Hash":
        return ""

    # Initialize result string
    result = ""
    current_ops = ""


    # Handle FROM clause for scan operations
    if node_type in ("Seq Scan", "Index Scan"):
        table = plan.get("Relation Name", plan.get("Alias", "Unknown"))
        current_ops += f"{indent}FROM {table} -- Cost: {cost_display}\n"
        if "Filter" in plan:
            filter_expr = clean_expression(plan["Filter"])
            current_ops += f"{indent}|> WHERE {filter_expr} -- Cost: {cost_display}\n"

    # Handle filter for non-scan nodes
    elif "Filter" in plan:
        filter_expr = clean_expression(plan["Filter"])
        current_ops += f"{indent}|> WHERE {filter_expr} -- Cost: {cost_display}\n"

    # Handle other node types
    if node_type == "Aggregate":
        mode = plan.get("Partial Mode", "")
        output_raw = plan.get("Output", [])
        output_expr = ", ".join([clean_expression(o) for o in output_raw]) if isinstance(output_raw, list) else clean_expression(output_raw) if output_raw else "aggregate values"
        
        if mode == "Partial":
            current_ops += f"{indent}|> AGGREGATE PARTIAL {output_expr} -- Cost: {cost_display}\n"
        elif mode == "Finalize":
            current_ops += f"{indent}|> AGGREGATE FINALIZE {output_expr} -- Cost: {cost_display}\n"
        else:
            current_ops += f"{indent}|> AGGREGATE {output_expr} -- Cost: {cost_display}\n"

    elif node_type == "Gather":
        current_ops += f"{indent}|> GATHER -- Cost: {cost_display}\n"

    elif node_type == "Limit":
        limit_rows = plan.get("Plan Rows", 1)
        current_ops += f"{indent}|> LIMIT {limit_rows} -- Cost: {cost_display}\n"

    elif node_type == "Sort":
        keys = plan.get("Sort Key", [])
        cleaned_keys = [clean_expression(k) for k in keys]
        current_ops += f"{indent}|> ORDER BY {', '.join(cleaned_keys)} -- Cost: {cost_display}\n"

    elif node_type == "Incremental Sort":
        keys = plan.get("Sort Key", [])
        presorted_keys = plan.get("Presorted Key", [])
        cleaned_keys = [clean_expression(k) for k in keys]
        cleaned_presorted = [clean_expression(k) for k in presorted_keys]
        
        if presorted_keys:
            current_ops += f"{indent}|> INCREMENTAL SORT (presorted: {', '.join(cleaned_presorted)}) BY {', '.join(cleaned_keys)} -- Cost: {cost_display}\n"
        else:
            current_ops += f"{indent}|> INCREMENTAL SORT BY {', '.join(cleaned_keys)} -- Cost: {cost_display}\n"

    elif node_type == "Nested Loop":
        join_filter = plan.get("Join Filter", "")
        if join_filter:
            join_filter = clean_expression(join_filter)
            current_ops += f"{indent}|> NESTED LOOP ON {join_filter} -- Cost: {cost_display}\n"
        else:
            current_ops += f"{indent}|> NESTED LOOP -- Cost: {cost_display}\n"

    elif node_type == "Merge Join":
        cond = clean_expression(plan.get("Merge Cond", ""))
        if cond:
            current_ops += f"{indent}|> MERGE JOIN ON {cond} -- Cost: {cost_display}\n"
        else:
            current_ops += f"{indent}|> MERGE JOIN -- Cost: {cost_display}\n"

    elif node_type == "Hash Join":
        cond = clean_expression(plan.get("Hash Cond", ""))
        if cond:
            current_ops += f"{indent}|> HASH JOIN ON {cond} -- Cost: {cost_display}\n"
        else:
            current_ops += f"{indent}|> HASH JOIN -- Cost: {cost_display}\n"

    elif node_type == "Materialize":
        current_ops += f"{indent}|> MATERIALIZE -- Cost: {cost_display}\n"
    
    # Handle subplan
    if plan.get("Parent Relationship") == "SubPlan":
        subplan = plan.get("Subplan Name", "Unnamed SubPlan")
        result += f"|> SUBPLAN {subplan} -- Cost: {cost_display}\n"
    for child in children:
        if child:
            child_result = parse_plan(child, depth + 1)
            if child_result:
                result += child_result
    result += current_ops
    return result  # Always return the result string


def clean_expression(expr):
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
    return str(expr)  # Convert non-string to string


def convert_qep_to_pipe_syntax(json_str):
    if not json_str:  # Guard against None or empty string
        print("Empty input")
        return ""

    try:
        tree_json = json.loads(json_str)
        if not tree_json:  # Guard against empty JSON
            print("Empty JSON array")
            return ""

        first_item = tree_json[0]
        if not first_item:  # Guard against empty first item
            print("Empty first item")
            return ""

        if "Plan" in first_item:
            plan_root = first_item["Plan"]
        else:
            plan_root = first_item

        if not plan_root:  # Guard against empty plan root
            print("Empty plan root")
            return ""

        result = parse_plan(plan_root, max_depth=None)

        # Always return a string
        if not result:
            print("Empty result from parse_plan")
            return ""

        return result.rstrip()

    except json.JSONDecodeError as e:
        print(f"Error parsing JSON: {e}")
        return ""
    except KeyError as e:
        print(f"Missing expected key: {e}")
        return ""
    except Exception as e:
        print(f"Unexpected error: {e}")
        return ""


def main(preprocessed_plan):
    if preprocessed_plan:
        return convert_qep_to_pipe_syntax(preprocessed_plan)
    return None


if __name__ == "__main__":
    print("***************")
    try:
        with open("test.json") as f:
            json_str = f.read()

        if not json_str:
            print("Warning: Read empty file")

        result = convert_qep_to_pipe_syntax(json_str)
        print(result)

    except Exception as e:
        print(f"Error in main: {e}")
    print("***************")
