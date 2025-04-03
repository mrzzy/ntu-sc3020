import json 
import re

from_emitted = False

def parse_plan(plan):
  node_type = plan.get("Node Type", "Unknown")
  children = plan.get("Plans", [])
  startup_cost = plan.get("Startup Cost", "N/A")
  total_cost = plan.get("Total Cost", "N/A")
  cost_display = f"{startup_cost} → {total_cost}"
  lines = []
  
  # skip Hash node 
  if node_type == "Hash":
    return ""
  
  for child in children:
    child_result = parse_plan(child)
    if child_result:
      #lines.append(child_result)
      # issue in sql4 - FROM comes after ORDER BY - prepend it ?
      #lines += [child_result]  
      # below to fixed in sql5 multiple from at beginning
      lines.extend(child_result.split("\n"))
  
  cost_display = f"{total_cost}"
  
  # handle current node: only seq scan
  # 4/2 index scan added
  if node_type in ("Seq Scan", "Index Scan"):
      table = plan.get("Relation Name", plan.get("Alias", "Unknown"))

      global from_emitted
      if not from_emitted:
        lines.append(f"FROM {table} -- Cost: {cost_display}")
        from_emitted = True
         
      # Show WHERE filter if present (even if not root)
      if "Filter" in plan:
          filter_expr = plan["Filter"]
          filter_expr = (
              filter_expr.replace("::text", "")
              .replace("::bpchar", "")
              .replace("::date", "")
              .replace("::timestamp without time zone", "")
              .replace("::numeric", "")
              .replace("~~", "LIKE")
          )
          lines.append(f"|> WHERE {filter_expr} -- Cost: {total_cost}")


    
  # handle aggregate
  elif node_type == "Aggregate":
    mode = plan.get("Partial Mode", "")
    output_raw = plan.get("Output", [])
    if isinstance(output_raw, list):
      output_expr = ", ".join([clean_expression(o) for o in output_raw])
    else:
      output_expr = clean_expression(output_raw) if output_raw else "aggregate values"
      
    if mode == "Partial":
      lines.append(f"|> AGGREGATE PARTIAL {output_expr} -- Cost: {cost_display}")
    elif mode == "Finalize":
      lines.append(f"|> AGGREGATE FINALIZE {output_expr} -- Cost: {cost_display}")
    else:
      lines.append(f"|> AGGREGATE {output_expr} -- Cost: {cost_display}")
  # handle gather
  elif node_type == "Gather":
    lines.append(f"|> GATHER -- Cost: {cost_display}")
  # handle limit
  elif node_type == "Limit":
    limit_rows = plan.get("Plan Rows", 1)
  # handle sort
  elif node_type == "Sort":
    keys = plan.get("Sort Key", [])
    cleaned_keys = [clean_expression(k) for k in keys]
    lines.append(f"|> ORDER BY {', '.join(cleaned_keys)} -- Cost: {cost_display}")
  # handle join 
  elif node_type == "Nested Loop":
    lines.append(f"|> NESTED LOOP -- Cost: {cost_display}")
  elif node_type == "Merge Join":
    cond = clean_expression(plan.get("Merge Cond", ""))
    if cond:
      lines.append(f"|> MERGE JOIN ON {cond} -- Cost: {cost_display}")
    else:
      lines.append(f"|> MERGE JOIN -- Cost: {cost_display}")
  elif node_type == "Hash Join":
    cond = clean_expression(plan.get("Hash Cond", ""))
    if cond:
      lines.append(f"|> HASH JOIN ON {cond} -- Cost: {cost_display}")
    else:
      lines.append(f"|> HASH JOIN -- Cost: {cost_display}")
  
  # handle subplan 
  if plan.get("Parent Relationship") == "SubPlan":
    subplan = plan.get("Subplan Name", "Unnamed SubPlan")
    lines.append(f"|> SUBPLAN {subplan} -- Cost: {cost_display}")
    
  
  return "\n".join(lines)

# sql3 found filter will be in other node like sort 
# idea: apply it when processing Sort Key, Group Key, Output etc 
def clean_expression(expr):
  if isinstance(expr, str):
      return (
        expr.replace("::text", "")
        .replace("::bpchar", "")
        .replace("::date", "")
        .replace("::timestamp without time zone", "")
        .replace("::numeric", "")
        .replace("~~", "LIKE")
      )
  return expr

  
def convert_qep_to_pipe_syntax(json_str):
  try:
    tree_json = json.loads(json_str)
    first_item = tree_json[0]
    
    tree_json = json.loads(json_str)
    first_item = tree_json[0]
    
    if "Plan" in first_item:
        plan_root = first_item["Plan"]
    else:
        plan_root = first_item
    return parse_plan(plan_root)

  except json.JSONDecodeError:
      print("Error processing QEP tree JSON")
      return None
  except KeyError as e:
      print(f"Missing expected key: {e}")
      return None


  
  
if __name__ == "__main__":
  with open('/Users/zuozhiyi/Desktop/sc3020/ntu-sc3020/project2/qep_5.json') as f:
    # /Users/zuozhiyi/Desktop/sc3020/ntu-sc3020/project2/raw.json
    # /Users/zuozhiyi/Desktop/sc3020/ntu-sc3020/project2/qep_3.json
    json_str = f.read()
  print(convert_qep_to_pipe_syntax(json_str))