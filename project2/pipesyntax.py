import json 
import re

def parse_plan(plan, depth=0):
  if not plan: 
    return ""
    
  node_type = plan.get("Node Type", "Unknown")
  children = plan.get("Plans", [])
  startup_cost = plan.get("Startup Cost", "N/A")
  total_cost = plan.get("Total Cost", "N/A")
  cost_display = f"{startup_cost} : -> {total_cost}"  # Custom format
  
  # Skip Hash node 
  if node_type == "Hash":
    return ""
    
  # Initialize result string
  result = ""
    
  # Process child plans first 
  for child in children:
    if child:  # Ensure child is not None
      child_result = parse_plan(child, depth + 1)
      if child_result and isinstance(child_result, str):
        result += child_result
  
  # Handle FROM clause for scan operations
  if node_type in ("Seq Scan", "Index Scan"):
    table = plan.get("Relation Name", plan.get("Alias", "Unknown"))
    result += f"FROM {table} -- Cost: {cost_display}\n"
    
    # Handle WHERE clause if filter exists
    if "Filter" in plan:
      filter_expr = clean_expression(plan["Filter"])
      result += f"|> WHERE {filter_expr} -- Cost: {cost_display}\n"
    
  # Handle filter for non-scan nodes
  elif "Filter" in plan:
    filter_expr = clean_expression(plan["Filter"])
    result += f"|> WHERE {filter_expr} -- Cost: {cost_display}\n"
  
  # Handle other node types
  if node_type == "Aggregate":
    mode = plan.get("Partial Mode", "")
    output_raw = plan.get("Output", [])
    if isinstance(output_raw, list):
      output_expr = ", ".join([clean_expression(o) for o in output_raw])
    else:
      output_expr = clean_expression(output_raw) if output_raw else "aggregate values"
      
    if mode == "Partial":
      result += f"|> AGGREGATE PARTIAL {output_expr} -- Cost: {cost_display}\n"
    elif mode == "Finalize":
      result += f"|> AGGREGATE FINALIZE {output_expr} -- Cost: {cost_display}\n"
    else:
      result += f"|> AGGREGATE {output_expr} -- Cost: {cost_display}\n"
  
  elif node_type == "Gather":
    result += f"|> GATHER -- Cost: {cost_display}\n"
    
  elif node_type == "Limit":
    limit_rows = plan.get("Plan Rows", 1)
    result += f"|> LIMIT {limit_rows} -- Cost: {cost_display}\n"
    
  elif node_type == "Sort":
    keys = plan.get("Sort Key", [])
    cleaned_keys = [clean_expression(k) for k in keys]
    result += f"|> ORDER BY {', '.join(cleaned_keys)} -- Cost: {cost_display}\n"
  
  elif node_type == "Incremental Sort":
    keys = plan.get("Sort Key", [])
    presorted_keys = plan.get("Presorted Key", [])
    
    cleaned_keys = [clean_expression(k) for k in keys]
    cleaned_presorted = [clean_expression(k) for k in presorted_keys]
    
    if presorted_keys:
      result += f"|> INCREMENTAL SORT (presorted: {', '.join(cleaned_presorted)}) BY {', '.join(cleaned_keys)} -- Cost: {cost_display}\n"
    else:
      result += f"|> INCREMENTAL SORT BY {', '.join(cleaned_keys)} -- Cost: {cost_display}\n"
     
  elif node_type == "Nested Loop":
    join_filter = plan.get("Join Filter", "")
    if join_filter:
      join_filter = clean_expression(join_filter)
      result += f"|> NESTED LOOP ON {join_filter} -- Cost: {cost_display}\n"
    else:
      result += f"|> NESTED LOOP -- Cost: {cost_display}\n"
    
  elif node_type == "Merge Join":
    cond = clean_expression(plan.get("Merge Cond", ""))
    if cond:
      result += f"|> MERGE JOIN ON {cond} -- Cost: {cost_display}\n"
    else:
      result += f"|> MERGE JOIN -- Cost: {cost_display}\n"
      
  elif node_type == "Hash Join":
    cond = clean_expression(plan.get("Hash Cond", ""))
    if cond:
      result += f"|> HASH JOIN ON {cond} -- Cost: {cost_display}\n"
    else:
      result += f"|> HASH JOIN -- Cost: {cost_display}\n"
      
  # Handle Materialize
  elif node_type == "Materialize":
    result += f"|> MATERIALIZE -- Cost: {cost_display}\n"
  
  # Handle subplan 
  if plan.get("Parent Relationship") == "SubPlan":
    subplan = plan.get("Subplan Name", "Unnamed SubPlan")
    result += f"|> SUBPLAN {subplan} -- Cost: {cost_display}\n"
    
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
    
    result = parse_plan(plan_root)
    
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
  return None
if __name__ == "__main__":
  try:
    with open('/Users/zuozhiyi/Desktop/sc3020/ntu-sc3020/project2/qep_8.json') as f:
      json_str = f.read()
      
    if not json_str:
      print("Warning: Read empty file")
      
    result = convert_qep_to_pipe_syntax(json_str)
    print(result)
    
  except Exception as e:
    print(f"Error in main: {e}")